#include <math.h>
#include "C4.h"

#define LENGTH 512

float minFreq = 60.0;
const float sample_freq = 22050;

// short rawData[LENGTH];
int len = sizeof(rawData);
int count;
int i, k;
long sum, sum_old;
int thresh = 0;
float freq_per = 0;
short pd_state = 0;

void measureFrequency() {
	sum = 0;
	pd_state = 0;
	int period = 0;
	for(i = 0; i < len; i++) {
		sum_old = sum;
		sum = 0;
		for(k = 0; k < len-i; k++) sum += (rawData[k]-128)*(rawData[k+i]-128)/256;

		// Serial.println(sum);

		if(pd_state == 2 && (sum-sum_old) <= 0){
			period = i;
			pd_state = 3;
		}

		if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

		if(!i) {
			thresh = sum * 0.5;
			pd_state = 1;
		}

		// for(i=0; i < len; i++) Serial.println(rawData[i]);

		if(thresh > 100){
			freq_per = sample_freq/period;
			Serial.println(freq_per);
		}
		count = 0;
	}
}

void setup() {
	// analogReference(EXTERNAL);
	// TODO: May need to change this depending on what the ADC pin is on ESP32
	// analogRead(A0);
	Serial.begin(115200);
	delay(1000);
	count = 0;

	measureFrequency();
}

void loop () {
	
	// if(count < LENGTH) {
	// 	count++;
	// 	rawData[count] = analogRead(A0)>>2;
	// } else {
	// }
}
