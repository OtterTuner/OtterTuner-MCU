#define LENGTH 512

extern double tunings[6];
extern int string_number;
extern short rawData[LENGTH];
extern double desired_freq;

const float sample_freq = 16667;
int len = LENGTH;
int thresh = 0;
double freq_thres = 0.20 * desired_freq;

/*
 * String parsing globals
 */

/*
 * Indices are as follows:
 * E2 A2 D3 G3 B3 E4
 * 0  1  2  3  4  5
 */

void parseTuningString(String fullTuning) {
  char delimiter = ',';
  char closing = ']';

  int strLength = fullTuning.length();
  String tuning = "";
  int tuningIndex = 0;

  for(int i = 1; i < strLength; i++) {
    if(fullTuning[i] == delimiter || fullTuning[i] == closing) {
      //skip the comma and the space
      i += 1;
      tunings[tuningIndex] = tuning.toDouble();
      // Serial.printf("tuning at %d: %f\r\n", tuningIndex, tunings[tuningIndex]);
      tuningIndex++;
      tuning = "";
      continue;
    }

    tuning += fullTuning[i];
  }
}

int stringCheck(double frequency){
	if(frequency>70 && frequency<90){ return 0; }
	if(frequency>100 && frequency<120){ return 1; }
	if(frequency>135 && frequency<155){ return 2; }
	if(frequency>186 && frequency<205){ return 3; }
	if(frequency>235 && frequency<255){ return 4; }
	if(frequency>320 && frequency<340){ return 5; }
}

/*
* 	TUNING SENSING
*/
double measureFrequency() {
	int sum = 0;
	static int sum_old = 0;
	static short pd_state = 0;
	int period = 0;
	double measured_freq;

	for(int attempts = 0; attempts < 32; attempts++){
		for(int i = 0; i < len; i++) {
			sum_old = sum;
			sum = 0;

			for(int k = 0; k < len-i; k++) sum += (rawData[k]-2048)*(rawData[k+i]-2048)/4096;

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

		if(period != 0 && thresh > 70) {
			// offset by 4 because we're bad lol
			measured_freq = sample_freq/period - 4;
			double discrepancy = abs(desired_freq - measured_freq);

			if(discrepancy < freq_thres) {
				Serial.printf("measured_frequency: %f\r\n", measured_freq);
				return measured_freq;
			}
		}
	}
  return -1;
}

