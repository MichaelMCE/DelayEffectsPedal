


#ifndef _NOTE_H_
#define _NOTE_H_



#define NOTE_OCTAVES			6		// number of octaves covered


enum _notes {
	NOTE_C = 0,
	NOTE_C_SHARP,
	NOTE_D,
	NOTE_D_SHARP,
	NOTE_E,
	NOTE_F,
	NOTE_F_SHARP,
	NOTE_G,
	NOTE_G_SHARP,
	NOTE_A,
	NOTE_A_SHARP,
	NOTE_B
};

typedef struct {
	uint8_t note;
	char label[3];
	float freq[NOTE_OCTAVES];
}notetable_t;


void note_init (const float standardA4);

float noteFindNearest (float freqIn, uint8_t *noteOut, uint8_t *octaveOut);

const float noteToFreq (const uint8_t note, const uint8_t octave);
const char *noteToLabel (const uint8_t note);

float noteGetLower (uint8_t *note, uint8_t *octave);
float noteGetHigher (uint8_t *note, uint8_t *octave);

#endif
