#include <math.h>
#include <string>
#include <Preferences.h>
#include <driver/adc.h>

#define DEVICE_NAME "OtterTuner"
#define LENGTH 4000
#define TUNING_BUTTON_PIN 10

Preferences preferences;
double desired_freq;
double sample_freq;
short rawData[LENGTH];

/*
 * Indices for string numbers are as follows:
 * E2 A2 D3 G3 B3 E4
 * 0  1  2  3  4  5
 */

double tunings[6] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};
int string_number;

double get_tuning(){
	preferences.begin(DEVICE_NAME, true);
	String tuning = preferences.getString("tuning", "");
	parseTuningString(tuning);
	preferences.end();

	return tunings[string_number];
}

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

void setup() {
    Serial.begin(115200);
    motorSetup();
    bluetooth_init();
    adc_setup();
    buttonSetup();
    sample_freq = 0;
    delay(3000);
    Serial.println("setup complete");
}

void loop() {
	// Serial_Monitor();
    
    // bool isTuningOn = digitalRead( TUNING_BUTTON_PIN );
    bool isTuningOn = false;
    
    if( isTuningOn ) {
        double startTime = millis();
        for (int i = 0; i < LENGTH; i++) {
            rawData[i] = adc1_get_raw(ADC1_CHANNEL_4);
        }
        double endTime = millis();
        sample_freq = (LENGTH / (endTime - startTime)) * 1000;

        double prev_freq = desired_freq;

        desired_freq = get_tuning();
        double current_frequency = measureFrequency(sample_freq);
        Serial.printf("current_frequency: %d, desired freq: %f\r\n", current_frequency, desired_freq);

        pid(current_frequency);
    // } else {
        // TODO: Insert battery reading code
    }
}
