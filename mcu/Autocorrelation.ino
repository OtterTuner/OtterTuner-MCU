#define OFFSET          2048
#define MAX_ADC_VALUE   4096

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

double measureFrequency(double sample_freq) {
    int sum = 0;
    static int sum_old = 0;
    static short pd_state = 0;
    int period = 0;
    double measured_freq;

    for(int i = 0; i < len; i++) {
        sum_old = sum;
        sum = 0;

        // calculate autocorrelation. samples must be offset by 2048 to ensure "zero" value is 2048.
        // Samples are also normalized to ensure the autocorrelation is scaled properly.
        for(int k = 0; k < len-i; k++) sum += (rawData[k]-OFFSET)*(rawData[k+i]-OFFSET)/MAX_ADC_VALUE;

        // Autocorrelation is decreasing from the peak, which means we've detected a period
        if(pd_state == 2 && (sum-sum_old) <= 0){
            period = i;
            pd_state = 3;
        }

        // We've reached the threshold and the autocorrelation is increasing
        if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;
        
        // Ensure that we don't accidentally take a period at the beginning of the sample
        // set threshold to indicate when we should begin checking for period
        if(!i) {
            thresh = sum * 0.5;
            pd_state = 1;
        }
    }

    // Ensure that we get a valid period
    if(period != 0) {
        measured_freq = sample_freq/period;
        freq_thres = 0.20 * desired_freq;

        double discrepancy = abs(desired_freq - measured_freq);

        if(discrepancy < freq_thres) {
            Serial.printf("Measured frequency: %f, Desired Frequency: %f\r\n", measured_freq, desired_freq);
            return measured_freq;
        }
    }

    return -1;
}

