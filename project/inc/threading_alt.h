/* add user code begin Header */
/**
  **************************************************************************
  * @file     threading_alt.h
  * @brief    mbedtls threading type for MBEDTLS_THREADING_ALT (FreeRTOS)
  **************************************************************************
  * mbedtls includes this file from mbedtls/threading.h when
  * MBEDTLS_THREADING_ALT is defined, and expects the definition of
  * mbedtls_threading_mutex_t here.
  *
  * The handle is kept as a void * on purpose so that FreeRTOS headers do not
  * have to be pulled into every mbedtls translation unit.  The matching
  * callbacks (recursive mutex create/take/give/delete) live in
  * project/src/mbedtls_port.c and are installed by mbedtls_port_init().
  **************************************************************************
  */
/* add user code end Header */

#ifndef MBEDTLS_THREADING_ALT_H
#define MBEDTLS_THREADING_ALT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mbedtls_threading_mutex_t
{
    void *handle;    /* SemaphoreHandle_t of a FreeRTOS recursive mutex */
    char  is_valid;  /* non zero once handle has been created          */
} mbedtls_threading_mutex_t;

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_THREADING_ALT_H */
