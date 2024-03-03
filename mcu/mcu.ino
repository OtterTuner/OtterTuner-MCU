#include <math.h>
#include <string>
#include <Preferences.h>
#include <driver/adc.h>

#define DEVICE_NAME "OtterTuner"
#define LENGTH 4000
#define TUNING_BUTTON_PIN   8
#define STRING_SWITCH_PIN   9

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

volatile bool buttonInterrupt = false;

void IRAM_ATTR ISR() {
    buttonInterrupt = true;
}

double get_tuning(){
	preferences.begin(DEVICE_NAME, true);
	String tuning = preferences.getString("tuning", "");
	parseTuningString(tuning);
	preferences.end();

	return tunings[string_number];
}

void setup_batteryADC() {

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
    int isTuningOn = digitalRead( TUNING_BUTTON_PIN );
    
    if( buttonInterrupt ) {
        buttonHandler();
        buttonInterrupt = false;
    } else if( isTuningOn == HIGH ) {
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
    } else {
        // TODO: Insert battery reading code
        int batteryVoltage = analogRead(ADC1_CHANNEL_MAX);
        // Serial.printf("Battery Voltage: %d\r\n", batteryVoltage);
    }
}
