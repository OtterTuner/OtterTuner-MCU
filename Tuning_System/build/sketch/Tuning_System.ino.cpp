#include <Arduino.h>
#line 1 "/Users/workryan/Documents/Arduino/Tuning_System/Tuning_System.ino"
#include <math.h>
#include "yinacf.h"

YinACF<float> yin;
float minFreq = 60.0;

void initYin(float sampleRate, float minFreq) {
	unsigned w, tmax;
	w = (unsigned)ceil(sampleRate/minFreq);
	tmax = w;
	yin.build(w, tmax);

}

int getFundamentalFrequency(int n, float* inSamples, float* outFrequencies) {
	int i;

	for(i = 0; i < n; ++i) {
		outFrequencies[i] = yin.tick(inSamples[i]);
	}
	
	return 0;
}

void setup() {
		
}

void loop () {
	
}

