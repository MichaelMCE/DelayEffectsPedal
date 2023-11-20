

#include "Pedal.h"
#include "codec.h"


#define DWIDTH					TFT_WIDTH
#define DHEIGHT					TFT_HEIGHT
#define TUNER_FREQ_MIN			10.0f
#define TUNER_FREQ_MAX			1500.0f
#define TUNER_TOLERENCE			4.10f

#define TOOL_Pneuma				525
#define TOOL_Invincible			410
#define TOOL_FearInoculum		335



static const char *fontfiles[FONT_TOTAL] = {
	UFDIR"ravebold110.uf",
	UFDIR"ravebold90.uf",
	UFDIR"ravebold80.uf",
	UFDIR"teutonweiss-bold30.uf",
	UFDIR"albasuper64.uf",
	UFDIR"76london38.uf",
	UFDIR"76london50.uf",
	UFDIR"76london80.uf",
	UFDIR"consola24.uf"
};

static _ufont_t fonts[FONT_TOTAL];
static _ufont_surface_t *surface;
static int32_t sceneUpdateRequested;
static uint32_t uiPage = UI_PAGE_STARTUP;
static const ui_page_t *uiPages;

static Encoder myEnc(ENCODER_PIN_CLK, ENCODER_PIN_DT);
static volatile int encPosNew = 0;
static volatile int encSwChange = 0;
static int encPos = -1;




void dumpStats ()
{

		
	printf("\r\n");
	printf("delay: %.2f %.2f\r\n", delayObj.processorUsage(), delayObj.processorUsageMax());
	printf("tuner: %.2f %.2f\r\n", tunerObj.processorUsage(), tunerObj.processorUsageMax());
	printf("flange: %.2f %.2f\r\n", flangeObj.processorUsage(), flangeObj.processorUsageMax());
	printf("chorus: %.2f %.2f\r\n", chorusObj.processorUsage(), chorusObj.processorUsageMax());
	printf("reverb: %.2f %.2f\r\n", reverbObj.processorUsage(), reverbObj.processorUsageMax());
		
	delayObj.processorUsageMaxReset();
	tunerObj.processorUsageMaxReset();
	flangeObj.processorUsageMaxReset();
	chorusObj.processorUsageMaxReset();
	reverbObj.processorUsageMaxReset();
}

static _ufont_t *getFont (const int fontIdx)
{
	return &fonts[fontIdx];
}

static void delaySet (const int period)
{
	ui_opaque_delay *delay = (ui_opaque_delay*)uiGetOpaque(UI_PAGE_DELAY);
	
	delay->period = period;
	delayObj.delay(0, delay->period);
}

static void delayStart ()
{
	ui_opaque_delay *delay = (ui_opaque_delay*)uiGetOpaque(UI_PAGE_DELAY);
	
	// defaults
	delay->step = 5;
	delay->state = 0;
	delay->period = 0;

	delaySet(TOOL_Pneuma);
}

static void delayEnable ()
{
	ui_opaque_delay *delay = (ui_opaque_delay*)uiGetOpaque(UI_PAGE_DELAY);
	
	delaySet(delay->period);
	outMixerObj.gain(1, 1.0f);	// delay
	delay->state = 1;
}

static void delayDisable ()
{
	ui_opaque_delay *delay = (ui_opaque_delay*)uiGetOpaque(UI_PAGE_DELAY);
	
	outMixerObj.gain(1, 0.0f);	// delay
	delayObj.disable(0);
	delay->state = 0;
}

static int delayStatus ()
{
	ui_opaque_delay *delay = (ui_opaque_delay*)uiGetOpaque(UI_PAGE_DELAY);
	
	return delay->state;
}

static void delayEncChange (const int direction)
{
	ui_opaque_delay *delay = (ui_opaque_delay*)uiGetOpaque(UI_PAGE_DELAY);
	
	if (direction < 0)
		delay->period -= delay->step;
	else
		delay->period += delay->step;
	
	if (delay->period < delay->step)
		delay->period = delay->step;
	else if (delay->period > 1000 - delay->step)
		delay->period = 1000 - delay->step;
}

static void encReset ()
{
	encPosNew = 0;
}

static void ufont_init ()
{
	// create a 1BPP work surface. this is the initial work surface
	surface = fontCreateSurface(DWIDTH, DHEIGHT, COLOUR_24TO16(0xFFBF33), NULL);
	
	// This will fail if memory is short, SDCard is unreadable
	// but more probable, requested font file (.uf) is absent
	for (int i = 0; i < (int)sizeof(fontfiles) / (int)sizeof(*fontfiles); i++){
		_ufont_t *font = getFont(i);
		if (fontOpen(font, fontfiles[i])){
			fontSetDisplayBuffer(font, tft_getBuffer(), DWIDTH, DHEIGHT);
			fontSetRenderSurface(font, surface);
			if (i != FONT_STARTUP1 && i != FONT_TUNER38 && i != FONT_TUNER50 && i != FONT_TUNER80 && i != FONT_CONSOLA24)
				fontSetGlyphPadding(font, fontGetGlyphPadding(font)+1);
		}
	}
}

static void audio_init ()
{
	// allocate enough memory for the delayObj
	AudioMemory(220);	// 2.9ms * 220blocks = max period
  
	// enable the audio shield
	codecObj.enable();
	codecObj.volume(0.9f); 
	codecObj.inputSelect(AUDIO_INPUT_LINEIN);
	codecObj.lineInLevel(10);
	codecObj.lineOutLevel(31);
	
	// disable delayObj taps 1 through 7
	for (int i = 1; i < 8; i++)
		delayObj.disable(i);
}

void encUpdate (const int value)
{
	if (!(value&0x03)){
		if (value != encPos){
			encPosNew = encPos - value;
			encPos = value;
		}
	}
}

static void encSwCB ()
{
	static int lastPressTime;
	
	int currentPressTime = millis();
	if (currentPressTime - lastPressTime > 400){
		lastPressTime = currentPressTime;
		encSwChange = 1;
	}
}

static void pins_init ()
{
	pinMode(LED_BUILTIN, OUTPUT);
	
	pinMode(ENCODER_PIN_SW, INPUT_PULLUP);
	attachInterrupt(ENCODER_PIN_SW, encSwCB, FALLING);
}

static void sceneStartupDraw (void *opaque)
{
	_ufont_t *font = getFont(FONT_STARTUP1);
	fontSetRenderFlags(font, BFONT_RENDER_WORDWRAP|BFONT_RENDER_NEWLINE|BFONT_RENDER_ADVANCE_X|BFONT_RENDER_ADVANCE_Y);
	
	char text[64];
	int x = 10;
	int y = 60;
	snprintf(text, sizeof(text), "Pneuma %i\r\nInvincible %i\r\n", TOOL_Pneuma, TOOL_Invincible);
	fontPrint(font, &x, &y, (uint8_t*)text);
	
	x = 10;
	snprintf(text, sizeof(text), "Fear Inoculum %i", TOOL_FearInoculum);
	fontPrint(font, &x, &y, (uint8_t*)text);

	fontApplySurface(font, 0, 0);
}

void sceneDelayDraw (void *opaque)
{
	char timestr[8];

	ui_opaque_delay *delay = (ui_opaque_delay*)opaque;
	sprintf(timestr, "%i", (int)delay->period);
	
	_ufont_t *font = getFont(FONT_DELAY);

	int width = 0;
	int height = 0;
	fontGetMetrics(font, (uint8_t*)timestr, &width, &height);
		
	int x = (abs(surface->width - width)/2) - 2;
	int y = (abs(surface->height - height)/2) - 4;
	fontPrint(font, &x, &y, (uint8_t*)timestr);
		
	//x = (DWIDTH - surface->width)/2;
	//y = (DHEIGHT - surface->height)/2;
	fontApplySurface(font, 0, 0);
}

void sceneTunerDraw (void *opaque)
{
	_ufont_t *font38 = getFont(FONT_TUNER38);
	_ufont_t *font50 = getFont(FONT_TUNER50);
	_ufont_t *font80 = getFont(FONT_TUNER80);
	_ufont_t *font24 = getFont(FONT_CONSOLA24);
	ui_opaque_tuner *tuner = (ui_opaque_tuner*)opaque;


	const int graphWidth = DWIDTH - 10;
	const float centScale = tuner->centRange;
	float notef = tuner->note;

	// note freq
	int width = 0;
	int height = 0;
	char notesText[32];
	sprintf(notesText, "%.1f", notef);
	fontGetMetrics(font38, (uint8_t*)notesText, &width, &height);
		
	int x = (abs(surface->width - width)/2);
	int y = 15;
	fontPrint(font38, &x, &y, (uint8_t*)notesText);
	
	
	// calc surrounding notes
	uint8_t noteNearest;
	uint8_t octaveNearest;
	float delta = noteFindNearest(notef, &noteNearest, &octaveNearest);
	float freqN = noteToFreq(noteNearest, octaveNearest);

	uint8_t noteOutL = noteNearest;
	uint8_t octaveOutL = octaveNearest;
	float freqL = noteGetLower(&noteOutL, &octaveOutL);

	uint8_t noteOutH = noteNearest;
	uint8_t octaveOutH = octaveNearest;
	float freqH = noteGetHigher(&noteOutH, &octaveOutH);


	// calc note cent off from nearest note
	float cent = 0.0f;
	if (delta > 0)
		cent = ((1.0f / (freqH - freqN)) * delta) * 100.0f;
	else if (delta < 0)
		cent = -((1.0f / (freqL - freqN)) * delta) * 100.0f;


	// note labels
	// lower note
	x = 4;
	y = 76;
	snprintf(notesText, sizeof(notesText), "%s", noteToLabel(noteOutL));
	fontPrint(font50, &x, &y, (uint8_t*)notesText);

	// nearest note
	snprintf(notesText, sizeof(notesText), "%s", noteToLabel(noteNearest));
	fontGetMetrics(font80, (uint8_t*)notesText, &width, &height);
	x = ((DWIDTH - width) / 2) - 2;
	y = 46;
	fontPrint(font80, &x, &y, (uint8_t*)notesText);

	// higher note
	snprintf(notesText, sizeof(notesText), "%s", noteToLabel(noteOutH));
	fontGetMetrics(font50, (uint8_t*)notesText, &width, &height);
	x = (DWIDTH - width) - 4;
	y = 76;
	fontPrint(font50, &x, &y, (uint8_t*)notesText);
	
	// horizontal mark
	x = (DWIDTH - graphWidth) / 2;
	y = 132;
	surfaceDrawLine(surface, x, y, x+graphWidth, y, 1);
	
	// left mark
	surfaceDrawLine(surface, x, y-2, x, y+4, 1);
	surfaceDrawLine(surface, x+1, y-2, x+1, y+4, 1);

	// middle mark
	surfaceDrawLine(surface, x+(graphWidth/2)-1, y-2, x+(graphWidth/2)-1, y+4, 1);
	surfaceDrawLine(surface, x+(graphWidth/2)-0, y-2, x+(graphWidth/2)-0, y+4, 1);
	surfaceDrawLine(surface, x+(graphWidth/2)+1, y-2, x+(graphWidth/2)+1, y+4, 1);
		
	// right mark
	surfaceDrawLine(surface, x+graphWidth-1, y-2, x+graphWidth-1, y+4, 1);
	surfaceDrawLine(surface, x+graphWidth, y-2, x+graphWidth, y+4, 1);

	// cent labels
	snprintf(notesText, sizeof(notesText), "-%.0f", centScale/2.0f);
	x = 2;
	y = 132 + 6;
	fontPrint(font24, &x, &y, (uint8_t*)notesText);

	snprintf(notesText, sizeof(notesText), "%.0f", centScale/2.0f);
	fontGetMetrics(font24, (uint8_t*)notesText, &width, &height);
	x = (DWIDTH - width) - 5;
	y = 132 + 6;
	fontPrint(font24, &x, &y, (uint8_t*)notesText);


	// indicator
	float centX = (float)(graphWidth / centScale) * cent;
	centX += (graphWidth / 2.0f);
	centX += (DWIDTH - graphWidth) / 2.0f;
	x = 5;
	y = 132;

	if (fabsf(cent) > TUNER_TOLERENCE)
		surfaceDrawTriangle(surface, x+(graphWidth/2)-13, DHEIGHT-2, x+(graphWidth/2)+15, DHEIGHT-2, centX, y+5, 1);
	else
		surfaceDrawTriangleFilled(surface, x+(graphWidth/2)-13, DHEIGHT-2, x+(graphWidth/2)+15, DHEIGHT-2, centX, y+5, 1);

	// cents
	int xOffset = x + (graphWidth * 0.16f);
	y = 166;
	snprintf(notesText, sizeof(notesText), "%.0f", fabsf(cent));
	fontPrint(font38, &xOffset, &y, (uint8_t*)notesText);	

	// octave
	xOffset = x + (graphWidth * 0.72f);
	y = 166;
	snprintf(notesText, sizeof(notesText), "%i", octaveNearest);
	fontPrint(font38, &xOffset, &y, (uint8_t*)notesText);
	
	fontApplySurface(font38, 0, 0);
}

void sceneFlangeDraw (void *opaque)
{
	//ui_opaque_flange *flange = (ui_opaque_flange*)opaque;
	_ufont_t *font = getFont(FONT_EFFECT1);
	fontSetRenderFlags(font, BFONT_RENDER_WORDWRAP|BFONT_RENDER_NEWLINE|BFONT_RENDER_ADVANCE_X|BFONT_RENDER_ADVANCE_Y);
	
	char text[64];
	int x = 15;
	int y = 60;
	snprintf(text, sizeof(text), "Flange");
	fontPrint(font, &x, &y, (uint8_t*)text);

	fontApplySurface(font, 0, 0);
}

void sceneChorusDraw (void *opaque)
{
	//ui_opaque_chorus *chorus = (ui_opaque_chorus*)opaque;
	_ufont_t *font = getFont(FONT_EFFECT1);
	fontSetRenderFlags(font, BFONT_RENDER_WORDWRAP|BFONT_RENDER_NEWLINE|BFONT_RENDER_ADVANCE_X|BFONT_RENDER_ADVANCE_Y);
	
	char text[64];
	int x = 15;
	int y = 60;
	snprintf(text, sizeof(text), "Chorus");
	fontPrint(font, &x, &y, (uint8_t*)text);
	fontApplySurface(font, 0, 0);
}

void sceneReverbDraw (void *opaque)
{
	ui_opaque_reverb *reverb = (ui_opaque_reverb*)opaque;
	_ufont_t *font = getFont(FONT_EFFECT1);
	fontSetRenderFlags(font, BFONT_RENDER_WORDWRAP|BFONT_RENDER_NEWLINE|BFONT_RENDER_ADVANCE_X|BFONT_RENDER_ADVANCE_Y);
	
	char text[64];
	int x = 15;
	int y = 50;
	snprintf(text, sizeof(text), "Reverb\r\n");
	fontPrint(font, &x, &y, (uint8_t*)text);

	x = 32;
	y += 20;
	snprintf(text, sizeof(text), "%.2f", reverb->room);
	fontPrint(font, &x, &y, (uint8_t*)text);

	fontApplySurface(font, 0, 0);
}

static void sceneUpdate ()
{
	sceneUpdateRequested = 1;
}

static void tunerReset ()
{
	ui_opaque_tuner *tuner = (ui_opaque_tuner*)uiGetOpaque(UI_PAGE_TUNER);
	tuner->note = 0.0f;
}

static void tunerEnable ()
{
	tunerObj.enable();
}

static void tunerDisable ()
{
	tunerObj.disable();
}

static void tunerStart ()
{
	ui_opaque_tuner *tuner = (ui_opaque_tuner*)uiGetOpaque(UI_PAGE_TUNER);
	tuner->note = 0.0f;
	tuner->centRange = 100.0f;
	tunerObj.begin(0.10f);
}

static int tunerUpdate ()
{
	ui_opaque_tuner *tuner = (ui_opaque_tuner*)uiGetOpaque(UI_PAGE_TUNER);
	
	int updated = tunerObj.available();
	if (updated){
		tuner->note = tunerObj.read();
		if (tuner->note < TUNER_FREQ_MIN)
			tuner->note = 0.01f;
		else if (tuner->note > TUNER_FREQ_MAX)
			tuner->note = TUNER_FREQ_MAX;
	}
	return updated;
}

static void flangeStart ()
{
	#define FLANGE_DELAY_LENGTH (6*AUDIO_BLOCK_SAMPLES)
	
	static short flangeDelayline[FLANGE_DELAY_LENGTH];
	

	int s_idx = FLANGE_DELAY_LENGTH/4;
	int s_depth = FLANGE_DELAY_LENGTH/4;
	double s_freq = 0.5f;

	flangeObj.begin(flangeDelayline, FLANGE_DELAY_LENGTH, s_idx, s_depth, s_freq);
	flangeObj.voices(s_idx, s_depth, s_freq);
}

static void flangeEnable ()
{
	flangeObj.enable();
}

static void flangeDisable ()
{
	flangeObj.disable();
}
	
static void chorusStart ()
{
	ui_opaque_chorus *chorus = (ui_opaque_chorus*)uiGetOpaque(UI_PAGE_CHORUS);
	
	static short chorusDelayline[CHORUS_DELAY_LENGTH];
	

	chorus->voices = 2;
	chorusObj.begin(chorusDelayline, CHORUS_DELAY_LENGTH, chorus->voices);
	chorusObj.voices(chorus->voices);
}

static void chorusEnable ()
{
	ui_opaque_chorus *chorus = (ui_opaque_chorus*)uiGetOpaque(UI_PAGE_CHORUS);
	
	chorusObj.voices(chorus->voices);
	chorusObj.enable();
}

static void chorusDisable ()
{
	chorusObj.disable();
	chorusObj.voices(0);
}

static void reverbStart ()
{
	ui_opaque_reverb *reverb = (ui_opaque_reverb*)uiGetOpaque(UI_PAGE_REVERB);
	reverb->room = 0.5f;
	reverb->damping = 0.05f;
	reverb->step = 0.05f;
	
	reverbObj.roomsize(reverb->room);
	reverbObj.damping(reverb->damping);
}

static void reverbEnable ()
{
	reverbObj.enable();
}

static void reverbDisable ()
{
	reverbObj.disable();
}

static float reverbEncChange (const float direction)
{
	ui_opaque_reverb *reverb = (ui_opaque_reverb*)uiGetOpaque(UI_PAGE_REVERB);
	
	if (direction > 0.0f)
		reverb->room += reverb->step;
	else
		reverb->room -= reverb->step;

	if (reverb->room > 1.0f) reverb->room = 1.0f;
	else if (reverb->room < reverb->step) reverb->room = reverb->step;
	
	return reverb->room;
}

static void reverbRoomSet (const float value)
{
	ui_opaque_reverb *reverb = (ui_opaque_reverb*)uiGetOpaque(UI_PAGE_REVERB);
	reverb->room = value;
	reverbObj.roomsize(reverb->room);
}

static void sceneTestDraw (void *opaque)
{
	_ufont_t *font = getFont(FONT_EFFECT1);
	fontSetRenderFlags(font, BFONT_RENDER_WORDWRAP|BFONT_RENDER_NEWLINE|BFONT_RENDER_ADVANCE_X|BFONT_RENDER_ADVANCE_Y);
	
	char text[64];
	int x = 15;
	int y = 60;
	snprintf(text, sizeof(text), "Test");
	fontPrint(font, &x, &y, (uint8_t*)text);
	
	fontApplySurface(font, 0, 0);
}

void sceneTestEnter (void *opaque)
{
}

void sceneTestExit (void *opaque)
{
}

void sceneStartupEnter (void *opaque)
{
}

void sceneStartupExit (void *opaque)
{
}

void sceneDelayEnter (void *opaque)
{
	delayEnable();
}

void sceneDelayExit (void *opaque)
{
	delayDisable();
}

void sceneTunerEnter (void *opaque)
{
	tunerEnable();
	tunerReset();
}

void sceneTunerExit (void *opaque)
{
	tunerDisable();
}

void sceneFlangeEnter (void *opaque)
{
	flangeEnable();
}

void sceneFlangeExit (void *opaque)
{
	flangeDisable();
}

void sceneChorusEnter (void *opaque)
{
	chorusEnable();
}

void sceneChorusExit (void *opaque)
{
	chorusDisable();
}

void sceneReverbEnter (void *opaque)
{
	reverbEnable();
}

void sceneReverbExit (void *opaque)
{
	reverbDisable();
}

static void uiSetView (const int view)
{
	uiPage = view;
}

const int uiGetView ()
{
	return uiPage;
}

static void uiViewEnter (void *opaque)
{
	const int viewId = uiGetView();
	uiPages[viewId].func.enter(opaque);
}

static void uiViewRender (void *opaque)
{
	const int viewId = uiGetView();
	uiPages[viewId].func.draw(opaque);
}

static void uiViewExit (void *opaque)
{
	const int viewId = uiGetView();
	uiPages[viewId].func.exit(opaque);
}

void *uiGetOpaque (const int pageId)
{
	for (int i = 0; i < UI_PAGE_TOTAL; i++){
		if (uiPages[i].id == pageId)
			return uiPages[i].opaque;
	}
	return NULL;
}

static void *uiSetViewNext ()
{
	int view = uiGetView();
	
	if (++view >= UI_PAGE_TOTAL)
		view = 0;
		
	uiSetView(view);
	return uiGetOpaque(view);
}

static void sceneRender (void *opaque)
{
	fontCleanSurface(NULL, surface);
	tft_clearBuffer(COLOUR_BLACK);
	uiViewRender(opaque);
	tft_update();
}

static int uiDispatchMessage (const int eventId, uint32_t data1Uint32, int32_t data2Int32, float data3Flt)
{
	return uiEventCB(eventId, uiGetOpaque(uiGetView()), data1Uint32, data2Int32, data3Flt);
}

int uiEventCB (const int eventId, void *opaque, uint32_t data1uint32, int32_t data2Int32, float data3Flt)
{
	switch (eventId){
	  case UI_EVENT_ROTARY:
	    if (data2Int32 != IN_ROTARY_1) break;

		if (uiGetView() == UI_PAGE_DELAY){
			delayEncChange(data3Flt);
			
			ui_opaque_delay *delay = (ui_opaque_delay*)opaque;
			delaySet(delay->period);
			if (!delayStatus())		// delaySet() may reenable delay, ensure it remains off.
				delayDisable();
			return 1;
		}else if (uiGetView() == UI_PAGE_REVERB){
			reverbRoomSet(reverbEncChange(data3Flt));
			return 1;
			
		}else if (uiGetView() == UI_PAGE_TUNER){
			ui_opaque_tuner *tuner = (ui_opaque_tuner*)opaque;
			if (data3Flt > 0)
				tuner->centRange += 10.0f;
			else
				tuner->centRange -= 10.0f;
				
			if (tuner->centRange < 10.0f)
				tuner->centRange = 10.0f;
			return 1;
		}
		break;
	  case UI_EVENT_BUTTON:
	    if (data2Int32 == IN_SWITCH_1){
			uiDispatchMessage(UI_EVENT_PAGE_EXT, 0, 0, 0.0f);
			uiDispatchMessage(UI_EVENT_PAGE_NXT, 0, 0, 0.0f);
			uiDispatchMessage(UI_EVENT_PAGE_ENT, 0, 0, 0.0f);
	    	return 1;
	    }
		break;
	  case UI_EVENT_PAGE_NXT:
	    uiSetViewNext();
	    break;
	  case UI_EVENT_PAGE_ENT:
	  	uiViewEnter(opaque);
		break;
	  case UI_EVENT_PAGE_DRW:
	  	sceneRender(opaque);
		break;
	  case UI_EVENT_PAGE_EXT:
	  	uiViewExit(opaque);
		break;
	  case UI_EVENT_TICK:
		if (uiGetView() == UI_PAGE_TUNER)
			return tunerUpdate();
		break;
	};
	return 0;
}

static void ui_init ()
{
	static ui_opaque_t opaque;
	static const ui_page_t _uiPages[UI_PAGE_TOTAL] = {
		{UI_PAGE_INFO,   &opaque.info,   {sceneStartupEnter,sceneStartupDraw,sceneStartupExit}},
		{UI_PAGE_TUNER,  &opaque.tuner,  {sceneTunerEnter,  sceneTunerDraw,  sceneTunerExit}},
		{UI_PAGE_DELAY,  &opaque.delay,  {sceneDelayEnter,  sceneDelayDraw,  sceneDelayExit}},
		{UI_PAGE_FLANGE, &opaque.flange, {sceneFlangeEnter, sceneFlangeDraw, sceneFlangeExit}},
		{UI_PAGE_CHORUS, &opaque.chorus, {sceneChorusEnter, sceneChorusDraw, sceneChorusExit}},
		{UI_PAGE_REVERB, &opaque.reverb, {sceneReverbEnter, sceneReverbDraw, sceneReverbExit}},
		{UI_PAGE_test,   &opaque.test,   {sceneTestEnter,   sceneTestDraw,   sceneTestExit}}
	};
	
	
	uiPages = _uiPages;
	uiSetView(UI_PAGE_STARTUP);
}

void setup ()
{
	//Serial.begin(9600);

	tft_init();
	sd_init();
	audio_init();
	ufont_init();
	pins_init();
	ui_init();
	note_init();
	
	delayStart();
	delayDisable();
	
	tunerStart();
	tunerDisable();
	tunerReset();

	flangeStart();
	flangeDisable();
	
	chorusStart();
	chorusDisable();
	
	reverbStart();
	reverbDisable();
	
	sceneUpdate();
}

void loop ()
{
	if (encPosNew){
		if (uiDispatchMessage(UI_EVENT_ROTARY, 0, IN_ROTARY_1, encPosNew))
			sceneUpdate();
		encReset();
	}

	if (encSwChange){
		if (uiDispatchMessage(UI_EVENT_BUTTON, 0, IN_SWITCH_1, encSwChange))
			sceneUpdate();
		encSwChange = 0;
	}
	
	if (uiDispatchMessage(UI_EVENT_TICK, 0, 0, 0.0f))
		sceneUpdate();

	if (sceneUpdateRequested){
		uiDispatchMessage(UI_EVENT_PAGE_DRW, 0, 0, 0.0f);
		sceneUpdateRequested = 0;
	}

#if 1
	delay(1);
#else
	dumpStats();
	delay(30);
#endif
}

