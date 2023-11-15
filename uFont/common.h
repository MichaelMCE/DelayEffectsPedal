#pragma once
#ifndef _COMMON_H_
#define _COMMON_H_



#define USE_OLED		0

#define BPP_8			8
#define BPP_12			12
#define BPP_15			15
#define BPP_16			16
#define BPP_24			24
#define BPP_32			32

#define FIO_READ		((uint8_t)0x01)
#define FIO_WRITE		((uint8_t)0x02)
#define FIO_NEW			((uint8_t)0x04)
#define fileio_t		FsFile



#include <SD.h>
#include "libuf/ufont.h"
#include "libuf/pngcommon.h"
#include "libuf/ufont_primitives.h"
#include "config.h"
#include "displays.h"



#if USE_OLED
#define OLED_WIDTH		128
#define OLED_HEIGHT		64
#define OLED_ADDRESS	0x3C
#define OLED_WIREOBJ	(Wire2)	// T4.1 pins 24 and 25.
#include "I2C/SSD1306/ssd1306drv.h"
#endif





bool sd_init ();
fileio_t *fio_open (const uint8_t *filename, const uint8_t fio_flag);
size_t fio_read (fileio_t *fp, void *buffer, const size_t len);
void fio_close (fileio_t *fp);
size_t fio_seek (fileio_t *fp, size_t pos);

int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp);
int32_t png_read (const char *filename, uint8_t *buffer, const int bufferbpp, int32_t ox, int32_t oy);
int32_t png_readEx (const char *filename, uint8_t *buffer, const uint32_t bufferPitch, const int32_t bufferbpp, int32_t ox, int32_t oy);

#endif

