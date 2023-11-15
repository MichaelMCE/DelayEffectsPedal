

#include "Pedal.h"





static notetable_t notetable[12] = {
 {NOTE_C,       "C ", {}},
 {NOTE_C_SHARP, "C#", {}},
 {NOTE_D,       "D ", {}},
 {NOTE_D_SHARP, "D#", {}},
 {NOTE_E,       "E ", {}},
 {NOTE_F,       "F ", {}},
 {NOTE_F_SHARP, "F#", {}},
 {NOTE_G,       "G ", {}},
 {NOTE_G_SHARP, "G#", {}},
 {NOTE_A,       "A ", {}},
 {NOTE_A_SHARP, "A#", {}},
 {NOTE_B,       "B ", {}},
};

#if 0
void printnotes ()
{
	for (int o = 0; o < NOTE_OCTAVES; o++){
		printf(" Octave %i\n", o);
		for (int n = 0; n < 12; n++){
			float freq = notetable[n].freq[o];
			char *label = notetable[n].label;
			printf("%s - %.2f\n", label, freq);
		}
		printf("\n");
	}
}
#endif

const char *noteToLabel (const uint8_t note)
{
	return notetable[note].label;
}

const float noteToFreq (const uint8_t note, const uint8_t octave)
{
	return notetable[note].freq[octave];
}

float noteFindNearest (float freqIn, uint8_t *noteOut, uint8_t *octaveOut)
{
	float delta = 999999.0f;
	uint8_t note = -1;
	uint8_t octave = -1;
	
	for (int n = 0; n < 12; n++){
		for (int o = 0; o < NOTE_OCTAVES; o++){
			const float diff = fabsf(noteToFreq(n, o) - freqIn);
			if (diff < delta){
				delta = diff;
				note = n;
				octave = o;
			}
		}
	}
	
	if (note != -1){
		*noteOut = note;
		*octaveOut = octave;
	
		if (freqIn > noteToFreq(note, octave))
			return delta;
		else
			return -delta;
	}else{
		*noteOut = -1;
		return 0.0f;
	}
}

float noteGetLower (uint8_t *note, uint8_t *octave)
{
	int8_t octaveLower = *octave;
	int8_t noteLower = *note - 1;
		
	if (noteLower < NOTE_C){
		noteLower = NOTE_B;
		if (--octaveLower < 0)
			noteLower = -1;
	}

	if (noteLower != -1){
		float freq = noteToFreq(noteLower, octaveLower);
		*note = noteLower;
		*octave = octaveLower;
		//printf("freq l %.1f, %i %i\n", freq, *note, *octave);
		return freq;
	}
	
	return 0.0f;
}

float noteGetHigher (uint8_t *note, uint8_t *octave)
{
	int8_t octaveHigher = *octave;
	int8_t noteHigher = *note + 1;
	
	if (noteHigher > NOTE_B){
		noteHigher = NOTE_C;
		if (++octaveHigher >= NOTE_OCTAVES){
			octaveHigher = -1;
			noteHigher = -1;
		}
	}

	if (noteHigher != -1){
		float freq = noteToFreq(noteHigher, octaveHigher);
		*note = noteHigher;
		*octave = octaveHigher;
		//printf("freq h %.1f, %i %i\n", freq, *note, *octave);
		return freq;
	}

	return 0.0f;
}

static void buildOctave (float pitchStandard, int octave)
{
	octave -= 4;
	if (octave < -4){
		return;
	}else if (octave < 0){
		for (int d = octave+1; d <= 0; d++)
			pitchStandard /= 2.0f;
	}else if (octave > 0){
		for (int d = 0; d < octave; d++)
			pitchStandard *= 2.0f;
	}

	int noteIdx = 11;
	
	// A# and B
	for (float n = 2; n >= 1; n -= 1.0f){
		float freq = pitchStandard * exp2f(n/12.0f);
		//printf("freq %.1f\n", freq);
		notetable[noteIdx--].freq[octave+4] = freq;
	}
		
	// C to A
	for (float n = 0; n <= 9; n += 1.0f){
		float freq = pitchStandard / exp2f(n/12.0f);
		//printf("freq %.1f\n", freq);
		notetable[noteIdx--].freq[octave+4] = freq;
	}
}

static void noteBuildTable (const float A4)
{
	for (int o = 0; o < NOTE_OCTAVES; o++){
		//printf("# %i\n", o);
		buildOctave(A4, o);
		//printf("\n");
	}
}

void note_init ()
{
	noteBuildTable(NOTE_STANDARD);
}
