#include <math.h>
#include <string>
#include <Preferences.h>
#include <driver/adc.h>

#define DEVICE_NAME "OtterTuner"
#define LENGTH 4000

double tunings[6] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};
int string_number;
double desired_freq;
double sample_freq;
int count = 0;
short rawData[LENGTH];
int len = LENGTH;
int thresh = 0;
double freq_thres = 0.20 * desired_freq;

double measureFrequency(double sample_freq) {
    int sum = 0;
    static int sum_old = 0;
    static short pd_state = 0;
    int period = 0;
    double measured_freq;

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

        double discrepancy = abs(desired_freq - measured_freq);

        if(discrepancy < freq_thres) {
            return measured_freq;
        }
    }

    return -1;
}

void adc_setup(){
    adc_digi_init_config_t config;
	config.max_store_buf_size = 1024;
	config.adc1_chan_mask = BIT(6);
	config.adc2_chan_mask = 0;
	config.conv_num_each_intr = 256;

	adc_digi_pattern_config_t adc_pattern;
	adc_pattern.atten = ADC_ATTEN_DB_0;
	adc_pattern.channel = ADC1_CHANNEL_6;
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

void setup() {
	Serial.begin(115200);
  adc_setup();
  sample_freq = 0;
	Serial.println("setup complete");
}

void loop() {
  int in = Serial.read();
  if (in == 'n') {
      string_number = (string_number + 1) % 6;
      desired_freq = tunings[string_number];
      Serial.printf("string number: %d\r\n", string_number);
  } else if (in == 'p') {
      string_number = (string_number - 1) % 6;
      desired_freq = tunings[string_number];
      Serial.printf("string number: %d\r\n", string_number);
  }

  double startTime = millis();
  for (int i = 0; i < LENGTH; i++) {
      rawData[i] = adc1_get_raw(ADC1_CHANNEL_4);
  }
  double endTime = millis();
  sample_freq = (LENGTH / (endTime - startTime)) * 1000;

  double prev_freq = desired_freq;

  double current_frequency = measureFrequency(sample_freq);

	Serial.println(current_frequency);

  count = 0;
}
