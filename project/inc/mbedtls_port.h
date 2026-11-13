/* add user code begin Header */
/**
  **************************************************************************
  * @file     mbedtls_port.h
  * @brief    mbedtls port layer API (AT32 TRNG entropy source + FreeRTOS)
  **************************************************************************
  * Typical use, e.g. inside a FreeRTOS task:
  *
  *   #include "mbedtls_port.h"
  *
  *   static mbedtls_pk_context priv;
  *   static mbedtls_pk_context pub;
  *   unsigned char hash[32];
  *   unsigned char sig[MBEDTLS_PORT_ECDSA_DER_MAX];
  *   size_t sig_len;
  *   int ret;
  *
  *   ret = mbedtls_port_init();                  // once, before anything else
  *
  *   mbedtls_pk_init(&priv);
  *   ret = mbedtls_port_load_private_key_pem(privatekey_pem, 0, &priv);
  *
  *   mbedtls_pk_init(&pub);
  *   ret = mbedtls_port_load_public_key_pem(publickey_pem, 0, &pub);
  *
  *   ret = mbedtls_port_sha256(message, message_len, hash);
  *
  *   ret = mbedtls_port_ecdsa_sign(&priv, hash, sizeof(hash),
  *                                 sig, sizeof(sig), &sig_len);
  *
  *   ret = mbedtls_port_ecdsa_verify(&pub, hash, sizeof(hash), sig, sig_len);
  *   // ret == 0 : signature is valid
  *
  *   mbedtls_pk_free(&priv);
  *   mbedtls_pk_free(&pub);
  *
  * Notes
  *   - mbedtls_port_init() must run once, from a task, before any other
  *     mbedtls or mbedtls_port call (it installs the FreeRTOS mutexes, starts
  *     the TRNG and seeds CTR_DRBG).  The other functions fail with
  *     MBEDTLS_PORT_ERR_NOT_INITIALIZED instead of using an unseeded DRBG.
  *   - Signatures are ASN.1/DER encoded (mbedtls default), at most
  *     MBEDTLS_PORT_ECDSA_DER_MAX bytes for P-256.
  *   - PEM parsers need the whole PEM text in memory.  This project has no
  *     filesystem, so pass the bytes of publickey.pem / privatekey.pem read
  *     from flash (or a const string).  PEM data must be plain text, not
  *     encrypted.
  *   - These functions are not interrupt safe.
  **************************************************************************
  */
/* add user code end Header */

#ifndef MBEDTLS_PORT_H
#define MBEDTLS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>

#include "mbedtls/pk.h"

/* exported macro ------------------------------------------------------------*/
/* private includes 0 */
#define MBEDTLS_PORT_SHA256_LEN         ( 32 )
/* maximum DER encoded ECDSA signature size for secp256r1 */
#define MBEDTLS_PORT_ECDSA_DER_MAX      ( 72 )

/* port specific error codes (negative, no clash with mbedtls codes) */
#define MBEDTLS_PORT_ERR_NOT_INITIALIZED   ( -0x0001 )
#define MBEDTLS_PORT_ERR_NOT_EC_KEY        ( -0x0002 )
#define MBEDTLS_PORT_ERR_BAD_INPUT         ( -0x0003 )
#define MBEDTLS_PORT_ERR_HASH_LEN          ( -0x0004 )
#define MBEDTLS_PORT_ERR_SELFTEST_FAILED   ( -0x0005 )

/* mbedtls_port_selftest() result bits */
#define MBEDTLS_PORT_TEST_SHA256           ( 0x01 )  /* known answer mismatch   */
#define MBEDTLS_PORT_TEST_SIGN             ( 0x02 )  /* ECDSA sign failed       */
#define MBEDTLS_PORT_TEST_VERIFY           ( 0x04 )  /* valid signature refused */
#define MBEDTLS_PORT_TEST_TAMPER           ( 0x08 )  /* tampered signature kept */

/* exported functions ------------------------------------------------------- */

/* allocator handed to mbedtls through MBEDTLS_PLATFORM_STD_CALLOC/FREE */
void *mbedtls_port_calloc(size_t n, size_t size);
void mbedtls_port_free(void *ptr);

/* one time initialisation: mutexes, TRNG, CTR_DRBG.  @retval 0 on success. */
int mbedtls_port_init(void);

/* RNG callback for mbedtls (CTR_DRBG seeded from the TRNG). */
int mbedtls_port_random(void *ctx, unsigned char *output, size_t len);

/* SHA-256 of a whole buffer. */
int mbedtls_port_sha256(const uint8_t *data, size_t len,
                        uint8_t out[MBEDTLS_PORT_SHA256_LEN]);

/* Parse a private key.  pem_len may be 0 when pem is NUL terminated.
   pk must be initialised with mbedtls_pk_init() and must be freed with
   mbedtls_pk_free() by the caller, also on error. */
int mbedtls_port_load_private_key_pem(const char *pem, size_t pem_len,
                                      mbedtls_pk_context *pk);

/* Parse a public key.  Same contract as above. */
int mbedtls_port_load_public_key_pem(const char *pem, size_t pem_len,
                                     mbedtls_pk_context *pk);

/* ECDSA sign a SHA-256 digest (hash_len == 32), DER signature out.
   @retval 0 on success. */
int mbedtls_port_ecdsa_sign(mbedtls_pk_context *priv,
                            const uint8_t *hash, size_t hash_len,
                            uint8_t *der_sig, size_t der_sig_size,
                            size_t *der_sig_len);

/* Verify an ECDSA DER signature over a SHA-256 digest.
   @retval 0 when the signature is valid, a negative mbedtls error code when
   it is invalid or on failure. */
int mbedtls_port_ecdsa_verify(mbedtls_pk_context *pub,
                              const uint8_t *hash, size_t hash_len,
                              const uint8_t *der_sig, size_t der_sig_len);

/* On target check of SHA-256, TRNG/CTR_DRBG and ECDSA sign/verify.
   @retval 0 all checks passed
           >0 bit mask of MBEDTLS_PORT_TEST_* failures
           <0 mbedtls error code from setup (init or key generation) */
int mbedtls_port_selftest(void);

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_PORT_H */
