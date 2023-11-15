


#include "common.h"






int readPngToFrame (uint8_t *frame, png_structp *png_ptr, int bpp, int width, int height, int32_t ox, int32_t oy, int passCount, const uint32_t bufferPitch)
{
	uint8_t line[(width * 4) + 4];		// enough for a single RGBA line

	int ret = 1;
	switch (bpp){
	  //case BPP_8:  readPng32To8      (frame, png_ptr, width, height, line, ox, oy, passCount, bufferPitch); break;
	  //case BPP_12: readPng32To12     (frame, png_ptr, width, height, line, ox, oy, passCount, bufferPitch); break; // non packed 12bit
	  //case BPP_15: readPng32To16_555 (frame, png_ptr, width, height, line, ox, oy, passCount, bufferPitch); break;
	  case BPP_16: readPng32To16_565(frame, png_ptr, width, height, line, ox, oy, passCount, bufferPitch); break;
	  //case BPP_24: readPng32To24     (frame, png_ptr, width, height, line, ox, oy, passCount, bufferPitch); break;
	  //case BPP_32: readPng32To32A_cpy(frame, png_ptr, width, height, line, ox, oy, passCount, bufferPitch); break;
	  //default: printf("libpng: format %i not implemented\r\n", bpp); ret = 0;
	};

	return ret;
}

#if 0
void PNGAPI warningCallback (png_structp png_ptr, png_const_charp warning_msg)
{
	char *err = (char *)png_get_error_ptr(png_ptr);
	if (err)
		Serial.printf("libpng: warning (%s), %s\r\n",err, (intptr_t)warning_msg);
	else
		Serial.printf("libpng: warning %s\r\n",(intptr_t)warning_msg);
	
}
#endif

FsFile *pngfile;

void png_read_dataCb (png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check = (png_size_t)pngfile->read(data, length);
	if (check != length){
		//printf("read Error, length:%i\r\n", length);
	}
}

int32_t loadPng (uint8_t *frame, const int flags, const char *filename, int32_t ox, int32_t oy, uint32_t *width, uint32_t *height, uint32_t *bpp, uint32_t bufferPitch)
{
	//printf("loadPng(): #%s#\r\n", filename);

	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;


	pngfile = (FsFile*)calloc(1, sizeof(FsFile));
	if (!pngfile->open(filename, O_RDONLY)){
		//printf("loadPng(): open failed\r\n");
		free(pngfile);
		return 0;
	}
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		pngfile->close();
		free(pngfile);
		return 0;
	}

	png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
#if 0
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) warningCallback, warningCallback);
#endif
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		pngfile->close();
		free(pngfile);
		return 0;
	}

	png_set_read_fn(png_ptr, (png_voidp)pngfile, png_read_dataCb);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);

	int passCount = 1;
	int bit_depth = 0, color_type = 0, interlace_type = 0;
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)width, (png_uint_32*)height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	if (bpp) *bpp = bit_depth;
	
	if (frame != NULL){
		if (interlace_type == PNG_INTERLACE_ADAM7)
			passCount = png_set_interlace_handling(png_ptr);
			//printf("interlace_type %i, ct:%i, %i %i\n", interlace_type, passCount, *width, *height);

	
		if (bit_depth == 16)
			png_set_strip_16(png_ptr);
		png_set_packing(png_ptr);

		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);
		else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand_gray_1_2_4_to_8(png_ptr);

		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png_ptr);
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		
		/*if (flags&LOAD_RESIZE){
			if (!_resizeFrame(frame, *width, *height, 0))
				return 0;
		}*/
	}

	int ret = 1;
	if (frame){
		if (!bufferPitch) bufferPitch = (*width)<<1;
		ret = readPngToFrame(frame, &png_ptr, flags, *width, *height, ox, oy, passCount, bufferPitch);
	}
	
	if (!ret)
		png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
	pngfile->close();
	free(pngfile);

	return ret;
}

int32_t png_read (const char *filename, uint8_t *buffer, const int bufferbpp, int32_t ox, int32_t oy)
{
	uint32_t width = 0;
	uint32_t height = 0;
	
	return loadPng(buffer, bufferbpp, filename, ox, oy, &width, &height, NULL, 0);
}

int32_t png_readEx (const char *filename, uint8_t *buffer, const uint32_t bufferPitch, const int32_t bufferbpp, int32_t ox, int32_t oy)
{
	uint32_t width = 0;
	uint32_t height = 0;
	
	return loadPng(buffer, bufferbpp, filename, ox, oy, &width, &height, NULL, bufferPitch);
}

int32_t png_metrics (const char *filename, uint32_t *width, uint32_t *height, uint32_t *filebpp)
{
	*width = 0;
	*height = 0;
	*filebpp = 0;

	return loadPng(NULL, 0, filename, 0, 0, width, height, filebpp, 0);
}

