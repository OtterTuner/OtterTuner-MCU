// #include "Guitar_C5.h"
// #include "C4.h"
// #include "E2.h"
#include <driver/adc.h>

#define LENGTH 5000

const int sample_freq = SOC_ADC_SAMPLE_FREQ_THRES_HIGH/2;

short rawData[LENGTH];
int len = sizeof(rawData);

int count;
int i, k;
long sum, sum_old;
int thresh = 0;
float measured_freq = 0;
float desired_freq = 82.4;
short pd_state = 0;

int prevTime;
int currentTime;

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
			pd_state = 3;
		}

		if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

		if(!i) {
			thresh = sum * 0.5;
			pd_state = 1;
		}
	}

	if(thresh > 100){
		measured_freq = sample_freq/period;
		Serial.println(measured_freq);
	}
}

void setup() {
	// analogReference(EXTERNAL);
	// TODO: May need to change this depending on what the ADC pin is on ESP32
	delay(3000);
	Serial.begin(115200);

	Serial.println("Before initialization");

	adc_digi_init_config_t config;
	config.max_store_buf_size = 1024;
	config.adc1_chan_mask = BIT(6);
	config.adc2_chan_mask = 0;
	config.conv_num_each_intr = 256;

	adc_digi_pattern_config_t adc_pattern;
	adc_pattern.atten = ADC_ATTEN_DB_11;
	adc_pattern.channel = ADC1_CHANNEL_6;
	adc_pattern.unit = ADC_UNIT_1;
	adc_pattern.bit_width = 12;

	adc_digi_configuration_t controller_config;
	controller_config.conv_limit_en = 0;
	controller_config.conv_limit_num = 250;
	controller_config.pattern_num = 1;
	controller_config.adc_pattern = &adc_pattern;
	controller_config.sample_freq_hz = 60000;
	controller_config.conv_mode = ADC_CONV_SINGLE_UNIT_1;
	controller_config.format = ADC_DIGI_OUTPUT_FORMAT_TYPE2;

	{
		Serial.printf("adc_pattern.atten: %d\r\n", adc_pattern.atten);
		Serial.printf("adc_pattern.channel: %d\r\n", adc_pattern.channel);
		Serial.printf("adc_pattern.unit: %d\r\n", adc_pattern.unit);
		Serial.printf("adc_pattern.bit_width: %d\r\n", adc_pattern.bit_width);
		Serial.printf("controller_config.conv_limit_en: %d\r\n", controller_config.conv_limit_en);
		Serial.printf("controller_config.conv_limit_num: %d\r\n", controller_config.conv_limit_num);
		Serial.printf("controller_config.pattern_num: %d\r\n", controller_config.pattern_num);
		Serial.printf("controller_config.sample_freq_hz: %d\r\n", controller_config.sample_freq_hz);
		Serial.printf("controller_config.conv_mode: %d\r\n", controller_config.conv_mode);
		Serial.printf("controller_config.format: %d\r\n", controller_config.format);
	}
	esp_err_t errCode = adc_digi_initialize(&config);
	Serial.printf("After initialization: %d\r\n", errCode);
	errCode = adc_digi_controller_configure(&controller_config);
	Serial.printf("After controller configure: %d\r\n", errCode);

	adc_digi_start();
	Serial.println("After start");

	count = 0;
	currentTime = micros();
}

void loop () {
	if(count < 1000) {
		prevTime = currentTime;
		currentTime = micros();

		uint8_t buffer[256] = {0};
		memset(buffer, 0xcc, 256);
		uint32_t out_length = 0;
		ESP_ERROR_CHECK(adc_digi_read_bytes(buffer, 256, &out_length, 3000));

		count++;
	} else {
		Serial.println(currentTime - prevTime);
		count = 0;
	}

	// if(count < LENGTH) {
	// 	count++;
	// 	rawData[count] = adc1_get_raw(ADC1_CHANNEL_0)>>2;
	// } else {
	// 	measureFrequency();
	// 	count = 0;
	// }
}
