/* add user code begin Header */
/**
  **************************************************************************
  * @file     mbedtls_port.c
  * @brief    mbedtls 3.6.7 port layer for Lanetum_AFE_Boot
  **************************************************************************
  * This file provides everything mbedtls needs on this target:
  *   - mbedtls_hardware_poll() : entropy from the AT32 hardware TRNG
  *   - mbedtls_port_calloc/free: allocation from the FreeRTOS heap
  *   - FreeRTOS recursive mutexes for MBEDTLS_THREADING_ALT
  *   - SHA-256 / ECDSA(P-256) / PEM key loading helpers used by the
  *     application (see mbedtls_port.h)
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "wk_trng.h"
#include "mbedtls_port.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/sha256.h"
#include "mbedtls/threading.h"

/* private define ------------------------------------------------------------*/
/* Fail loudly if project/inc/mbedtls_config.h was not selected by the build.
   It is selected by the forwarder in
   middlewares/mbedtls-mbedtls-3.6.7/include/mbedtls/mbedtls_config.h, which
   build_info.h includes when MBEDTLS_CONFIG_FILE is not defined. */
#if !defined(MBEDTLS_ECDSA_C) || !defined(MBEDTLS_ENTROPY_HARDWARE_ALT) || \
    !defined(MBEDTLS_PEM_PARSE_C) || !defined(MBEDTLS_THREADING_ALT) || \
    !defined(MBEDTLS_PLATFORM_MEMORY)
#error "mbedtls configuration not selected: middlewares/mbedtls-mbedtls-3.6.7/include/mbedtls/mbedtls_config.h must forward to project/inc/mbedtls_config.h"
#endif

/* Number of TRNG register polls before mbedtls_hardware_poll() reports a
   source failure.  One iteration is only a few CPU cycles, so this covers
   far more than the time the TRNG needs to produce one 32 bit word. */
#define MBEDTLS_PORT_TRNG_POLL_LIMIT       ( 5000000UL )

/* private variables ---------------------------------------------------------*/
static int s_port_ready = 0;

static mbedtls_entropy_context s_entropy;
static mbedtls_ctr_drbg_context s_ctr_drbg;

/* private functions ---------------------------------------------------------*/

/**
  * @brief  allocate zeroed memory from the FreeRTOS heap.
  *         FreeRTOS heap_4 suspends the scheduler around the heap access, so
  *         this is safe to call from several tasks.
  * @param  n: number of elements
  * @param  size: size of one element
  * @retval pointer to the zeroed block, NULL on failure
  */
void *mbedtls_port_calloc(size_t n, size_t size)
{
    void *ptr;
    size_t total;

    if ((n == 0) || (size == 0))
    {
        return NULL;
    }

    total = n * size;
    if ((total / n) != size)          /* multiplication overflow */
    {
        return NULL;
    }

    ptr = pvPortMalloc(total);
    if (ptr != NULL)
    {
        memset(ptr, 0, total);
    }

    return ptr;
}

/**
  * @brief  release memory allocated by mbedtls_port_calloc().
  * @param  ptr: block to release, NULL is ignored
  * @retval none
  */
void mbedtls_port_free(void *ptr)
{
    if (ptr != NULL)
    {
        vPortFree(ptr);
    }
}

/**
  * @brief  create a recursive mutex for mbedtls.
  */
static void mbedtls_port_mutex_init(mbedtls_threading_mutex_t *mutex)
{
    if ((mutex == NULL) || (mutex->is_valid != 0))
    {
        return;                       /* already created: keep it working */
    }

    mutex->handle = (void *) xSemaphoreCreateRecursiveMutex();
    if (mutex->handle != NULL)
    {
        mutex->is_valid = 1;
    }
}

/**
  * @brief  delete a mutex created by mbedtls_port_mutex_init().
  */
static void mbedtls_port_mutex_free(mbedtls_threading_mutex_t *mutex)
{
    if ((mutex != NULL) && (mutex->is_valid != 0))
    {
        vSemaphoreDelete((SemaphoreHandle_t) mutex->handle);
    }

    if (mutex != NULL)
    {
        mutex->handle = NULL;
        mutex->is_valid = 0;
    }
}

/**
  * @brief  take a mutex, waiting forever.
  */
static int mbedtls_port_mutex_lock(mbedtls_threading_mutex_t *mutex)
{
    if ((mutex == NULL) || (mutex->is_valid == 0))
    {
        return MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
    }

    if (xSemaphoreTakeRecursive((SemaphoreHandle_t) mutex->handle,
                                portMAX_DELAY) != pdTRUE)
    {
        return MBEDTLS_ERR_THREADING_MUTEX_ERROR;
    }

    return 0;
}

/**
  * @brief  give a mutex taken by mbedtls_port_mutex_lock().
  */
static int mbedtls_port_mutex_unlock(mbedtls_threading_mutex_t *mutex)
{
    if ((mutex == NULL) || (mutex->is_valid == 0))
    {
        return MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
    }

    if (xSemaphoreGiveRecursive((SemaphoreHandle_t) mutex->handle) != pdTRUE)
    {
        return MBEDTLS_ERR_THREADING_MUTEX_ERROR;
    }

    return 0;
}

/**
  * @brief  wait for one 32 bit word from the hardware TRNG.
  * @param  value: pointer receiving the random word
  * @retval 0 on success, -1 when the TRNG reports a failure or times out
  */
static int mbedtls_port_trng_word(uint32_t *value)
{
    uint32_t spins = 0;

    while (trng_flag_get(TRNG_DTRDY_FLAG) != SET)
    {
        if ((trng_flag_get(TRNG_CKFES_FLAG) == SET) ||
            (trng_flag_get(TRNG_ESFES_FLAG) == SET))
        {
            /* clock or entropy source failure: clear and report */
            trng_flag_clear(TRNG_CKFES_FLAG);
            trng_flag_clear(TRNG_ESFES_FLAG);
            return -1;
        }

        if (++spins >= MBEDTLS_PORT_TRNG_POLL_LIMIT)
        {
            return -1;
        }
    }

    *value = trng_value_get();
    return 0;
}

/**
  * @brief  entropy callback for mbedtls (MBEDTLS_ENTROPY_HARDWARE_ALT).
  *         Fills the buffer with TRNG data.
  * @param  data: unused, mbedtls passes NULL
  * @param  output: destination buffer
  * @param  len: number of bytes to produce
  * @param  olen: number of bytes actually produced
  * @retval 0 on success, MBEDTLS_ERR_ENTROPY_SOURCE_FAILED on TRNG failure
  */
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len,
                          size_t *olen)
{
    size_t index = 0;

    (void) data;

    if ((output == NULL) || (olen == NULL))
    {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    while (index < len)
    {
        uint32_t word;
        uint32_t byte;

        if (mbedtls_port_trng_word(&word) != 0)
        {
            *olen = index;
            return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }

        for (byte = 0; (byte < 4UL) && (index < len); byte++)
        {
            output[index++] = (unsigned char) (word >> (8UL * byte));
        }
    }

    *olen = len;
    return 0;
}

/* exported functions ------------------------------------------------------- */

/**
  * @brief  one time initialisation of the mbedtls port.
  *         Must be called from a task, before any other mbedtls call.
  * @retval 0 on success, mbedtls error code otherwise
  */
int mbedtls_port_init(void)
{
    int ret;
    static const char personalization[] = "lanetum-afe-boot";

    if (s_port_ready != 0)
    {
        return 0;
    }

    /* FreeRTOS mutexes must be installed before the first mbedtls call */
    mbedtls_threading_set_alt(mbedtls_port_mutex_init,
                              mbedtls_port_mutex_free,
                              mbedtls_port_mutex_lock,
                              mbedtls_port_mutex_unlock);

    /* entropy source: enable the TRNG (idempotent, main.c already did it) */
    wk_trng_init();

    mbedtls_entropy_init(&s_entropy);
    mbedtls_ctr_drbg_init(&s_ctr_drbg);

    ret = mbedtls_ctr_drbg_seed(&s_ctr_drbg, mbedtls_entropy_func, &s_entropy,
                                (const unsigned char *) personalization,
                                sizeof(personalization) - 1);
    if (ret != 0)
    {
        return ret;
    }

    s_port_ready = 1;
    return 0;
}

/**
  * @brief  random generator callback (CTR_DRBG seeded from the TRNG).
  * @param  ctx: unused
  * @param  output: destination buffer
  * @param  len: number of bytes
  * @retval 0 on success, mbedtls error code otherwise
  */
int mbedtls_port_random(void *ctx, unsigned char *output, size_t len)
{
    (void) ctx;

    if (s_port_ready == 0)
    {
        return MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
    }

    return mbedtls_ctr_drbg_random(&s_ctr_drbg, output, len);
}

/**
  * @brief  SHA-256 over one buffer.
  * @param  data: input data
  * @param  len: input length
  * @param  out: 32 byte digest
  * @retval 0 on success
  */
int mbedtls_port_sha256(const uint8_t *data, size_t len,
                        uint8_t out[MBEDTLS_PORT_SHA256_LEN])
{
    if ((data == NULL) || (out == NULL))
    {
        return MBEDTLS_PORT_ERR_BAD_INPUT;
    }

    return mbedtls_sha256(data, len, out, 0);
}

/**
  * @brief  parse a PEM private key into a pk context.
  * @param  pem: PEM text
  * @param  pem_len: text length, 0 to use strlen(pem)
  * @param  pk: initialised context, freed by the caller
  * @retval 0 on success
  */
int mbedtls_port_load_private_key_pem(const char *pem, size_t pem_len,
                                      mbedtls_pk_context *pk)
{
    int ret;
    char *buffer;

    if ((pem == NULL) || (pk == NULL))
    {
        return MBEDTLS_PORT_ERR_BAD_INPUT;
    }

    if (s_port_ready == 0)
    {
        return MBEDTLS_PORT_ERR_NOT_INITIALIZED;
    }

    if (pem_len == 0)
    {
        pem_len = strlen(pem);
    }

    /* mbedtls PEM parsing needs a NUL terminated buffer */
    buffer = (char *) mbedtls_port_calloc(1, pem_len + 1);
    if (buffer == NULL)
    {
        return MBEDTLS_ERR_PK_ALLOC_FAILED;
    }
    memcpy(buffer, pem, pem_len);

    ret = mbedtls_pk_parse_key(pk, (const unsigned char *) buffer, pem_len,
                               NULL, 0, mbedtls_port_random, NULL);

    /* do not leave the key material in the temporary buffer */
    mbedtls_platform_zeroize(buffer, pem_len + 1);
    mbedtls_port_free(buffer);

    if ((ret == 0) && (mbedtls_pk_can_do(pk, MBEDTLS_PK_ECDSA) == 0))
    {
        return MBEDTLS_PORT_ERR_NOT_EC_KEY;
    }

    return ret;
}

/**
  * @brief  parse a PEM public key into a pk context.
  * @param  pem: PEM text
  * @param  pem_len: text length, 0 to use strlen(pem)
  * @param  pk: initialised context, freed by the caller
  * @retval 0 on success
  */
int mbedtls_port_load_public_key_pem(const char *pem, size_t pem_len,
                                     mbedtls_pk_context *pk)
{
    int ret;
    char *buffer;

    if ((pem == NULL) || (pk == NULL))
    {
        return MBEDTLS_PORT_ERR_BAD_INPUT;
    }

    if (s_port_ready == 0)
    {
        return MBEDTLS_PORT_ERR_NOT_INITIALIZED;
    }

    if (pem_len == 0)
    {
        pem_len = strlen(pem);
    }

    buffer = (char *) mbedtls_port_calloc(1, pem_len + 1);
    if (buffer == NULL)
    {
        return MBEDTLS_ERR_PK_ALLOC_FAILED;
    }
    memcpy(buffer, pem, pem_len);

    ret = mbedtls_pk_parse_public_key(pk, (const unsigned char *) buffer,
                                      pem_len);

    mbedtls_port_free(buffer);

    if ((ret == 0) && (mbedtls_pk_can_do(pk, MBEDTLS_PK_ECDSA) == 0))
    {
        return MBEDTLS_PORT_ERR_NOT_EC_KEY;
    }

    return ret;
}

/**
  * @brief  ECDSA sign a SHA-256 digest (P-256, DER encoded signature).
  * @param  priv: private key context
  * @param  hash: SHA-256 digest
  * @param  hash_len: must be MBEDTLS_PORT_SHA256_LEN
  * @param  der_sig: output buffer, MBEDTLS_PORT_ECDSA_DER_MAX bytes are enough
  * @param  der_sig_size: size of der_sig
  * @param  der_sig_len: receives the signature length
  * @retval 0 on success
  */
int mbedtls_port_ecdsa_sign(mbedtls_pk_context *priv,
                            const uint8_t *hash, size_t hash_len,
                            uint8_t *der_sig, size_t der_sig_size,
                            size_t *der_sig_len)
{
    if ((priv == NULL) || (hash == NULL) || (der_sig == NULL) ||
        (der_sig_len == NULL))
    {
        return MBEDTLS_PORT_ERR_BAD_INPUT;
    }

    if (hash_len != MBEDTLS_PORT_SHA256_LEN)
    {
        return MBEDTLS_PORT_ERR_HASH_LEN;
    }

    if (s_port_ready == 0)
    {
        return MBEDTLS_PORT_ERR_NOT_INITIALIZED;
    }

    if (mbedtls_pk_can_do(priv, MBEDTLS_PK_ECDSA) == 0)
    {
        return MBEDTLS_PORT_ERR_NOT_EC_KEY;
    }

    return mbedtls_pk_sign(priv, MBEDTLS_MD_SHA256, hash, hash_len,
                           der_sig, der_sig_size, der_sig_len,
                           mbedtls_port_random, NULL);
}

/**
  * @brief  ECDSA verify a DER signature over a SHA-256 digest.
  * @param  pub: public key context
  * @param  hash: SHA-256 digest
  * @param  hash_len: must be MBEDTLS_PORT_SHA256_LEN
  * @param  der_sig: DER encoded signature
  * @param  der_sig_len: signature length
  * @retval 0 when the signature is valid
  */
int mbedtls_port_ecdsa_verify(mbedtls_pk_context *pub,
                              const uint8_t *hash, size_t hash_len,
                              const uint8_t *der_sig, size_t der_sig_len)
{
    if ((pub == NULL) || (hash == NULL) || (der_sig == NULL))
    {
        return MBEDTLS_PORT_ERR_BAD_INPUT;
    }

    if (hash_len != MBEDTLS_PORT_SHA256_LEN)
    {
        return MBEDTLS_PORT_ERR_HASH_LEN;
    }

    if (mbedtls_pk_can_do(pub, MBEDTLS_PK_ECDSA) == 0)
    {
        return MBEDTLS_PORT_ERR_NOT_EC_KEY;
    }

    return mbedtls_pk_verify(pub, MBEDTLS_MD_SHA256, hash, hash_len,
                             der_sig, der_sig_len);
}

/**
  * @brief  on target check of SHA-256, TRNG/CTR_DRBG and ECDSA sign/verify.
  *         Generate a throw away P-256 key, sign a SHA-256 digest and check
  *         that a valid signature is accepted and a tampered one is rejected.
  * @retval 0 all checks passed, >0 bit mask of failed checks, <0 mbedtls
  *         error code from setup
  */
int mbedtls_port_selftest(void)
{
    static const unsigned char sha256_abc[MBEDTLS_PORT_SHA256_LEN] =
    {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    static const unsigned char message[] = "abc";

    uint8_t hash[MBEDTLS_PORT_SHA256_LEN];
    uint8_t signature[MBEDTLS_PORT_ECDSA_DER_MAX];
    size_t signature_len = 0;
    mbedtls_ecdsa_context ecdsa;
    int ret;
    int failed = 0;

    ret = mbedtls_port_init();
    if (ret != 0)
    {
        return ret;
    }

    /* 1. SHA-256 known answer test */
    ret = mbedtls_port_sha256(message, sizeof(message) - 1, hash);
    if ((ret != 0) || (memcmp(hash, sha256_abc, sizeof(sha256_abc)) != 0))
    {
        failed |= MBEDTLS_PORT_TEST_SHA256;
    }

    /* 2. TRNG -> CTR_DRBG -> ECDSA key generation, sign and verify */
    mbedtls_ecdsa_init(&ecdsa);

    ret = mbedtls_ecdsa_genkey(&ecdsa, MBEDTLS_ECP_DP_SECP256R1,
                               mbedtls_port_random, NULL);
    if (ret != 0)
    {
        mbedtls_ecdsa_free(&ecdsa);
        return ret;
    }

    ret = mbedtls_ecdsa_write_signature(&ecdsa, MBEDTLS_MD_SHA256,
                                        hash, sizeof(hash),
                                        signature, sizeof(signature),
                                        &signature_len,
                                        mbedtls_port_random, NULL);
    if (ret != 0)
    {
        failed |= MBEDTLS_PORT_TEST_SIGN;
    }
    else if (mbedtls_ecdsa_read_signature(&ecdsa, hash, sizeof(hash),
                                          signature, signature_len) != 0)
    {
        failed |= MBEDTLS_PORT_TEST_VERIFY;
    }
    else
    {
        /* a corrupted signature must be rejected (DER tag changed) */
        signature[0] = (uint8_t) (signature[0] ^ 0x01);
        if (mbedtls_ecdsa_read_signature(&ecdsa, hash, sizeof(hash),
                                         signature, signature_len) == 0)
        {
            failed |= MBEDTLS_PORT_TEST_TAMPER;
        }
    }

    mbedtls_ecdsa_free(&ecdsa);

    return failed;
}
