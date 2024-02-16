extern double tunings[6];
extern int string_number;
extern short rawData[LENGTH];
extern double desired_freq;

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
            tuningIndex++;
            tuning = "";
            continue;
        }

        tuning += fullTuning[i];
    }
}

double avg_buffer(){
    double sum = 0;
    for(int i = 0; i < len; i++) sum += rawData[i];
    return sum/len;
}

double max_buffer(){
    double max = 0;
    for(int i = 0; i < len; i++) if(rawData[i] > max) max = rawData[i];
    return max;
}

double min_buffer(){
    double min = 4096;
    for(int i = 0; i < len; i++) if(rawData[i] < min) min = rawData[i];
    return min;
}

/*
* 	TUNING SENSING
*/
double measureFrequency(double sample_freq) {
    int sum = 0;
    static int sum_old = 0;
    static short pd_state = 0;
    int period = 0;
    double measured_freq;
    double max_val = max_buffer();
    double min_val = min_buffer();
    double offset = (max_val + min_val) / 2;
    double normalize = (max_val - min_val);

    // Serial.printf("Max: %f, Min: %f, Offset: %f, Normalize: %f\r\n", max_val, min_val, offset, normalize);

    for(int i = 0; i < len; i++) {
        sum_old = sum;
        sum = 0;

        for(int k = 0; k < len-i; k++) sum += (rawData[k]-2048)*(rawData[k+i]-2048)/4096;

        if(pd_state == 2 && (sum-sum_old) <= 0){
            period = i;
            pd_state = 3;
        }

        if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

        if(!i) {
            thresh = sum * 0.5;
            pd_state = 1;
        }
    }

    if(period != 0) {
        measured_freq = sample_freq/period;
        freq_thres = 0.20 * desired_freq;
        double avg = avg_buffer();
        // Serial.printf("Average: %f\n", avg);
        Serial.printf("Measured frequency: %f, sample_frequency: %f\r\n", measured_freq, sample_freq);

        double discrepancy = abs(desired_freq - measured_freq);

        if(discrepancy < freq_thres) {
            // Serial.printf("Desired frequency: %f\n", desired_freq);
            return measured_freq;
        }
    }

    return -1;
}

