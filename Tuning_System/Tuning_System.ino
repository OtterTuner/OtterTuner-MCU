#include <math.h>
#include "yinacf.h"

#define LENGTH 512

YinACF<float> yin;
float minFreq = 60.0;
const float sample_freq = 8919;

short rawData[LENGTH];

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
	analogReference(EXTERNAL);
	// TODO: May need to change this depending on what the ADC pin is on ESP32
	analogRead(A0);
	Serial.begin(115200);
	count = 0;
}

void loop () {
	
	if(count < LENGTH) {
		count++;
		rawData[count] = analogRead(A0)>>2;
	} else {
		sum = 0;

	}
}
