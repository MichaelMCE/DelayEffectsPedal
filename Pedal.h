
#ifndef _PEDAL_H_
#define _PEDAL_H_

#include <Arduino.h>
#include "audio/Audio.h"
#include "encoder/encoder.h"
#include "uFont/common.h"
#include "note.h"




#define CHORUS_DELAY_LENGTH		(12*AUDIO_BLOCK_SAMPLES)



#define IN_ROTARY_1				1001
#define IN_ROTARY_2				1002
#define IN_SWITCH_1				1011
#define IN_SWITCH_2				1012

#define ENCODER_PIN_DT			30
#define ENCODER_PIN_CLK			31
#define ENCODER_PIN_SW			32



enum _uiEvents {
	UI_EVENT_ROTARY = 1,		// dataInt32=which, dataFlt=direction
	UI_EVENT_BUTTON,			// dataInt32=which.	includes internal rotary switch
	UI_EVENT_PAGE_ENT,			// 
	UI_EVENT_PAGE_DRW,			// 
	UI_EVENT_PAGE_EXT,			// 
	UI_EVENT_PAGE_NXT,		
	UI_EVENT_TICK			
};

enum _uiPages {
	UI_PAGE_INFO,
	UI_PAGE_TUNER,
	UI_PAGE_DELAY,
	UI_PAGE_FLANGE,
	UI_PAGE_CHORUS,
	UI_PAGE_REVERB,
	UI_PAGE_test,
	UI_PAGE_TOTAL,
	UI_PAGE_STARTUP	= UI_PAGE_INFO
};

enum _fonts {
	FONT_DELAY,
	FONT_TUNER1,
	FONT_TUNER2,
	FONT_STARTUP1,
	FONT_EFFECT1,
	FONT_TUNER38,
	FONT_TUNER50,
	FONT_TUNER80,
	FONT_CONSOLA24,
	FONT_TOTAL
};



typedef struct _uiview {
	void (*enter) (void *opaque);
	void (*draw)  (void *opaque);
	void (*exit)  (void *opaque);
}ui_view_t;

typedef struct _page {
	int id;
	void *opaque;
	ui_view_t func;
}ui_page_t;

typedef struct {
	int voices;
}ui_opaque_chorus;

typedef struct {
	int period;
	int step;
	int state;
}ui_opaque_delay;

typedef struct {
	float note;
	float centRange;		// scale rane. -25 to 25, -50 to 50, etc..
}ui_opaque_tuner;

typedef struct {
	float room;
	float damping;
	float step;
}ui_opaque_reverb;

typedef struct {
	int stub;
}ui_opaque_flange;

typedef struct {
	int stub;
}ui_opaque_info;

typedef struct {
	int stub;
}ui_opaque_test;


typedef struct {
	ui_opaque_chorus chorus;
	ui_opaque_delay delay;
	ui_opaque_reverb reverb;
	ui_opaque_tuner tuner;
	ui_opaque_flange flange;
	ui_opaque_info info;
	
	ui_opaque_test test;
}ui_opaque_t;


const int uiGetView ();
void *uiGetOpaque (const int pageId);
int uiEventCB (const int eventId, void *opaque, uint32_t data1uint32, int32_t data2Int32, float data3Flt);


#endif

