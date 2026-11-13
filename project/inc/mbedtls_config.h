/* add user code begin Header */
/**
  **************************************************************************
  * @file     mbedtls_config.h
  * @brief    mbedtls 3.6.7 build configuration for Lanetum_AFE_Boot
  **************************************************************************
  * Target : AT32F455 (Cortex-M4F) + FreeRTOS
  * Entropy: AT32 hardware TRNG, through mbedtls_hardware_poll()
  *          implemented in project/src/mbedtls_port.c
  *
  * This is the mbedtls configuration of Lanetum_AFE_Boot.  It is pulled in by
  * middlewares/mbedtls-mbedtls-3.6.7/include/mbedtls/mbedtls_config.h, which
  * forwards to this file; build_info.h includes that file whenever the
  * compiler symbol MBEDTLS_CONFIG_FILE is not defined.  No compiler define,
  * include path or other build option is needed to select this configuration.
  *
  * Do not add MBEDTLS_CONFIG_FILE to the project settings: its value has to be
  * a quoted file name and that name does not survive the trip into the
  * generated makefile (the macro ends up as a bare token and build_info.h
  * fails with "#include expects "FILENAME" or <FILENAME>").
  *
  * Modules enabled:
  *   - SHA-256
  *   - ECDSA over secp256r1 (P-256); bignum + ECP
  *   - PK / PEM parsing      -> publickey.pem / privatekey.pem
  *   - entropy + CTR_DRBG    -> TRNG as the only entropy source
  *   - FreeRTOS recursive mutexes (MBEDTLS_THREADING_ALT)
  *
  * Modules deliberately NOT enabled (not required by this application, they
  * would dominate the code size): TLS (SSL), X.509, RSA, PSA crypto,
  * filesystem I/O, encrypted PEM (PKCS#5/#8 encryption).
  *
  * See project/src/mbedtls_port.c for the port implementation and the usage
  * example in project/inc/mbedtls_port.h.
  **************************************************************************
  */
/* add user code end Header */

/* mbedtls_config.h */
#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include <stddef.h>

/*----------------------------------------------------------------------------
 * Platform
 *   Heap comes from the FreeRTOS heap (see mbedtls_port.c); the rest of the
 *   C library (printf, memcpy, ...) still comes from newlib.
 *--------------------------------------------------------------------------*/
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_STD_CALLOC                  mbedtls_port_calloc
#define MBEDTLS_PLATFORM_STD_FREE                    mbedtls_port_free

void *mbedtls_port_calloc(size_t n, size_t size);
void mbedtls_port_free(void *ptr);

/*----------------------------------------------------------------------------
 * Threading
 *   FreeRTOS recursive mutexes, see threading_alt.h and mbedtls_port.c.
 *--------------------------------------------------------------------------*/
#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_ALT

/*----------------------------------------------------------------------------
 * Hashes
 *--------------------------------------------------------------------------*/
#define MBEDTLS_SHA256_C

/*----------------------------------------------------------------------------
 * Bignum / ECP / ECDSA (secp256r1 == P-256)
 *--------------------------------------------------------------------------*/
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECDSA_C

/*----------------------------------------------------------------------------
 * ASN.1 / OID / PK / PEM: load publickey.pem and privatekey.pem
 *--------------------------------------------------------------------------*/
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_OID_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_MD_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C

/*----------------------------------------------------------------------------
 * Entropy / random generator
 *   The AT32 TRNG is the only entropy source: no platform (OS) entropy.
 *--------------------------------------------------------------------------*/
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_AES_C                    /* required by CTR_DRBG */
#define MBEDTLS_AES_ROM_TABLES           /* tables in flash, saves RAM   */

/*----------------------------------------------------------------------------
 * Diagnostics: mbedtls_strerror().  Drop this define to save ~8 KB of flash.
 *--------------------------------------------------------------------------*/
#define MBEDTLS_ERROR_C

#endif /* MBEDTLS_CONFIG_H */
