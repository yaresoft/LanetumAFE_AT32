/*
 * version.h
 *
 *  Created on: Sep 21, 2026
 *      Author: yiz
 */

#ifndef INC_VERSION_H_
#include <stdint.h>

#define VERNUM_MAJOR 1
#define VERNUM_MINOR 0
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define VERSION_STRING TOSTRING(VERNUM_MAJOR) "." TOSTRING(VERNUM_MINOR) "." TOSTRING(BUILD_NUMBER)

#define BUILD_TIME __DATE__" "__TIME__
#define IMAGE_MAGIC 0x57465645


typedef struct
{
        uint32_t magic;
        uint32_t size;
        uint32_t signtime;
        uint32_t buildnumber;
        uint8_t hash[32];
        uint8_t signature[64];
        uint8_t version[32];
        uint8_t buildtime[32];
}__attribute__((packed))  AppBinHeaderType;

void Get_Version(void);
#define INC_VERSION_H_



#endif /* INC_VERSION_H_ */
