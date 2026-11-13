/**
 * \file mbedtls_config.h
 *
 * \brief Forwarder to the Lanetum_AFE_Boot mbedtls configuration.
 *
 * mbedtls/build_info.h includes this file whenever the compiler symbol
 * MBEDTLS_CONFIG_FILE is not defined, so this include is what selects the
 * project configuration:
 *
 *     middlewares/mbedtls-mbedtls-3.6.7/include/mbedtls/mbedtls_config.h
 *                              ->  project/inc/mbedtls_config.h
 *
 * Selecting the configuration with a file include is deliberate: a compiler
 * define would have to carry a quoted file name, and the AT32 IDE writes
 * defined symbols into the generated makefile where the build shell removes
 * one level of quotes.  The macro then expands to a bare token and
 * build_info.h fails with:
 *     #include expects "FILENAME" or <FILENAME>
 *
 * The reference mbedtls configuration that used to live here can be restored
 * with
 *     git checkout -- \
 *       middlewares/mbedtls-mbedtls-3.6.7/include/mbedtls/mbedtls_config.h
 * but the project configuration must then be selected through a compiler
 * symbol instead:
 *     MBEDTLS_CONFIG_FILE=\"mbedtls_config.h\"      (backslashes included)
 */

#include "../../../../project/inc/mbedtls_config.h"
