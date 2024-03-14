#define OFFSET          2048
#define MAX_ADC_VALUE   4096

void adc_setup(){
    adc_digi_init_config_t config;
    config.max_store_buf_size = 1024;
    config.adc1_chan_mask = BIT(6);
    config.adc2_chan_mask = 0;
    config.conv_num_each_intr = 256;

    adc_digi_pattern_config_t adc_pattern;
    adc_pattern.atten = ADC_ATTEN_DB_0;
    adc_pattern.channel = ADC1_CHANNEL_4;
    adc_pattern.unit = ADC_UNIT_1;
    adc_pattern.bit_width = 12;

    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0);

    adc_digi_configuration_t controller_config;
    controller_config.conv_limit_en = 0;
    controller_config.conv_limit_num = 250;
    controller_config.pattern_num = 1;
    controller_config.adc_pattern = &adc_pattern;
    controller_config.sample_freq_hz = 60000;
    controller_config.conv_mode = ADC_CONV_SINGLE_UNIT_1;
    controller_config.format = ADC_DIGI_OUTPUT_FORMAT_TYPE2;
}

void getSamples(){
        double startTime = millis();
        for (int i = 0; i < LENGTH; i++) {
            rawData[i] = adc1_get_raw(ADC1_CHANNEL_4);
        }
        double endTime = millis();
        sample_freq = (LENGTH / (endTime - startTime)) * 1000;
}

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

double measureFrequency(double sample_freq) {
    int sum = 0;
    static int sum_old = 0;
    static short pd_state = 0;
    int period = 0;
    double measured_freq;
    int thresh = 0;

    for(int i = 0; i < LENGTH; i++) {
        sum_old = sum;
        sum = 0;

        // calculate autocorrelation. samples must be offset by 2048 to ensure "zero" value is 2048.
        // Samples are also normalized to ensure the autocorrelation is scaled properly.
        for(int k = 0; k < LENGTH-i; k++) sum += (rawData[k]-OFFSET)*(rawData[k+i]-OFFSET)/MAX_ADC_VALUE;

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
        int freq_thres = 0.30 * desired_freq;

        double discrepancy = abs(desired_freq - measured_freq);

        if(discrepancy < freq_thres) {
            Serial.printf("Measured frequency: %f, Desired Frequency: %f\r\n", measured_freq, desired_freq);
            return measured_freq;
        }
    }

    return -1;
}

