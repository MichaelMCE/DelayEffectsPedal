

#include "common.h"

#if USE_OLED

#include "I2C/SSD1306/ssd1306drv.cpp"


static SSD1306 STORAGETYPE oled = SSD1306(OLED_WIDTH, OLED_HEIGHT, &OLED_WIREOBJ, -1);
static uint8_t STORAGETYPE oled_buffer[CALC_PITCH_1(OLED_WIDTH)*OLED_HEIGHT];



void oled_clear ()
{
	memset(oled_buffer, 0, sizeof(oled_buffer));
}

void oled_update ()
{
	const int srcPitch = CALC_PITCH_1(OLED_WIDTH);
	oled.clearDisplay();
	
	for (int y = 0; y < OLED_HEIGHT; y++){
		for (int x = 0; x < OLED_WIDTH; x++){
			if (getPixel1(oled_buffer, srcPitch, x, y))
				oled.drawPixel(x, y);
		}
	}
	oled.display();
}

uint8_t *oled_getBuffer ()
{
	return oled_buffer;
}

void oled_init ()
{
	oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
	oled_clear();
}


#endif

