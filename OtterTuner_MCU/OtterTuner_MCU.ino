// #include "Guitar_C5.h"
// #include "C4.h"
// #include "E2.h"

#define LENGTH 512

const int sample_freq = 16667;

short rawData[LENGTH];
int len = LENGTH;

int count;
int i, k;
long sum, sum_old;
int thresh = 0;
float measured_freq = 0;
float desired_freq = 82.4;
short pd_state = 0;

void measureFrequency() {
	sum = 0;
	pd_state = 0;
	int period = 0;

	for(i = 0; i < len; i++) {
		sum_old = sum;
		sum = 0;

		for(k = 0; k < len-i; k++) sum += (rawData[k]-128)*(rawData[k+i]-128)/256;

		if(pd_state == 2 && (sum-sum_old) <= 0){
			period = i;
			// Serial.printf("period: %d\n", period);
			pd_state = 3;
		}

		if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

		if(!i) {
			thresh = sum * 0.5;
			pd_state = 1;
		}
	}

	// if(thresh > 100){
	// 	measured_freq = sample_freq/period;
	// 	Serial.println(measured_freq);
	// }

	if(period != 0 && thresh > 100) {
		measured_freq = sample_freq/period;
		Serial.println(measured_freq);
	}
}

void setup() {
	Serial.begin(115200);
	delay(3000);
	count = 0;
	Serial.println("setup complete");
}

void loop () {
	if(count < LENGTH) {
		rawData[count] = analogRead(A0) >> 2;
		count++;
	} else {
		measureFrequency();
		count = 0;
	}
}
