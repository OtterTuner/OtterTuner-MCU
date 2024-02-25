#include <driver/adc.h>

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
  Serial.println("setup complete");
}

short data;
void loop() {
  data = adc1_get_raw(ADC1_CHANNEL_4);
  Serial.println(data);
  delay(1);
}
