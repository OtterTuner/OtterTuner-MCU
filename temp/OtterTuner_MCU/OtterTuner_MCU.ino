// #include "Guitar_C5.h"
// #include "C4.h"
// #include "E2.h"

#include <math.h>
#include <driver/adc.h>

#define LENGTH 1024
#define SAMPLE_RATE 16667.0

uint8_t rawData[LENGTH] = {0};
int len = LENGTH;

int count;
int i, k;
long sum, sum_old;
int thresh = 0;
double measured_freq = 0;
double valid_freq = 0;
double desired_freq = 82.4;
double freq_thres = 0.25 * desired_freq;
short pd_state = 0;

void measureFrequency() {
	sum = 0;
	pd_state = 0;
	int period = 0;
	int data = 0;
	int secondData;

	for(i = 0; i < len/sizeof(int); i++) {
		sum_old = sum;
		sum = 0;

		for(k = 0; k < len/4-i; k++) {
			data = 0;
			data |= rawData[i*4];
			data |= rawData[i*4 + 1] << 8;
			data |= rawData[i*4 + 2] << 16;
			data |= rawData[i*4 + 3] << 24;
			// Serial.printf("data: %d\r\n", data);

			secondData = 0;
			secondData |= rawData[(k+i)*4];
			secondData |= rawData[(k+i)*4 + 1] << 8;
			secondData |= rawData[(k+i)*4 + 2] << 16;
			secondData |= rawData[(k+i)*4 + 3] << 24;

		 	sum += (data-2048)*(secondData-2048)/4096;
		}

		if(pd_state == 2 && (sum-sum_old) <= 0){
			period = i*4;
			pd_state = 3;
		}

		if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

		if(!i) {
			thresh = sum * 0.5;
			pd_state = 1;
		}
	}

	// Serial.printf("%d\r\n", data);

	if(period != 0 && thresh > 1000) {
		measured_freq = SAMPLE_RATE/period;
		Serial.printf("%f\r\n", measured_freq);
		double discrepancy = abs(desired_freq - measured_freq);

		if(discrepancy < freq_thres) {
			valid_freq = measured_freq;
			// Serial.printf("%f\r\n", valid_freq);
		}
	}
}

void setup() {
	Serial.begin(115200);
	delay(3000);

	Serial.println("Before initialization");

	adc_digi_init_config_t config;
	config.max_store_buf_size = LENGTH*4;
	config.adc1_chan_mask = BIT(6);
	config.adc2_chan_mask = 0;
	config.conv_num_each_intr = LENGTH;

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
	controller_config.sample_freq_hz = SAMPLE_RATE;
	controller_config.conv_mode = ADC_CONV_SINGLE_UNIT_1;
	controller_config.format = ADC_DIGI_OUTPUT_FORMAT_TYPE2;

	// {
	// 	Serial.printf("adc_pattern.atten: %d\r\n", adc_pattern.atten);
	// 	Serial.printf("adc_pattern.channel: %d\r\n", adc_pattern.channel);
	// 	Serial.printf("adc_pattern.unit: %d\r\n", adc_pattern.unit);
	// 	Serial.printf("adc_pattern.bit_width: %d\r\n", adc_pattern.bit_width);
	// 	Serial.printf("controller_config.conv_limit_en: %d\r\n", controller_config.conv_limit_en);
	// 	Serial.printf("controller_config.conv_limit_num: %d\r\n", controller_config.conv_limit_num);
	// 	Serial.printf("controller_config.pattern_num: %d\r\n", controller_config.pattern_num);
	// 	Serial.printf("controller_config.SAMPLE_RATE_hz: %d\r\n", controller_config.sample_freq_hz);
	// 	Serial.printf("controller_config.conv_mode: %d\r\n", controller_config.conv_mode);
	// 	Serial.printf("controller_config.format: %d\r\n", controller_config.format);
	// }

	esp_err_t errCode = adc_digi_initialize(&config);
	// Serial.printf("After initialization: %d\r\n", errCode);
	errCode = adc_digi_controller_configure(&controller_config);
	// Serial.printf("After controller configure: %d\r\n", errCode);

	Serial.println("setup complete");
	adc_digi_start();
}

void loop () {
	memset(rawData, 0xcc, LENGTH);
	uint32_t out_length = 0;
	// Serial.println("Before read");
	ESP_ERROR_CHECK(adc_digi_read_bytes(rawData, LENGTH, &out_length, 3000));
	// Serial.println("After read");
	measureFrequency();
}
