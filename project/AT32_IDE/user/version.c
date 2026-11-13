/*
 * version.c
 *
 *  Created on: Sep 21, 2026
 *      Author: yiz
 */


#include "version.h"
extern unsigned int _image_size;




AppBinHeaderType __attribute__((section (".imgheader"))) ImageHeader={
		.magic=IMAGE_MAGIC,
		.size=(uint32_t)&_image_size,
		.version=VERSION_STRING,
		.buildtime=BUILD_TIME,
		.buildnumber=(BUILD_NUMBER)

};
void Get_Version(void)
{


}
