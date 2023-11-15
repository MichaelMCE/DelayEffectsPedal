
#ifndef _CODEC_H_
#define _CODEC_H_


/*

Update include.cpp to reflect any changes made here

To import back in to Teensy Audio Designer, remove the below keywords

*/

// GUItool: begin automatically generated code
AudioControlSGTL5000     codecObj;        //xy=277.23333740234375,29.99999237060547
AudioInputI2S            audioInObj;      //xy=73.23333740234375,124.99999237060547
AudioEffectFreeverb      reverbObj;       //xy=271.23333740234375,369.23333740234375
AudioEffectFlange        flangeObj;       //xy=272.23333740234375,288.99999237060547
AudioEffectChorus        chorusObj;       //xy=273.23333740234375,330.2333297729492
AudioAnalyzeNoteFrequency tunerObj;       //xy=275.23333740234375,81.99999237060547
AudioEffectDelay         delayObj;        //xy=275.23333740234375,193
AudioMixer4              effectMixerObj;  //xy=499.23333740234375,256.23333740234375
AudioMixer4              outMixerObj;     //xy=663.2333374023438,142
AudioOutputI2S           audioOutObj;     //xy=827.2333374023438,143

AudioConnection          patchCord1(audioInObj, 0, delayObj, 0);
AudioConnection          patchCord2(audioInObj, 0, flangeObj, 0);
AudioConnection          patchCord3(audioInObj, 0, outMixerObj, 0);
AudioConnection          patchCord4(audioInObj, 0, tunerObj, 0);
AudioConnection          patchCord5(audioInObj, 0, chorusObj, 0);
AudioConnection          patchCord6(audioInObj, 0, reverbObj, 0);
AudioConnection          patchCord7(reverbObj, 0, effectMixerObj, 2);
AudioConnection          patchCord8(flangeObj, 0, effectMixerObj, 0);
AudioConnection          patchCord9(chorusObj, 0, effectMixerObj, 1);
AudioConnection          patchCord10(delayObj, 0, outMixerObj, 1);
AudioConnection          patchCord11(effectMixerObj, 0, outMixerObj, 2);
AudioConnection          patchCord12(outMixerObj, 0, audioOutObj, 0);
AudioConnection          patchCord13(outMixerObj, 0, audioOutObj, 1);
// GUItool: end automatically generated code


#endif
