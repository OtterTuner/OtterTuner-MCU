#include <math.h>
#include <string>
#include <Preferences.h>
#include <driver/adc.h>

#define DEVICE_NAME "OtterTuner"
#define LENGTH 4000
#define TUNING_BUTTON_PIN   8
#define STRING_SWITCH_PIN   9
#define STABLE_THRESHOLD    10

Preferences preferences;
double desired_freq;
double sample_freq;
short rawData[LENGTH];
volatile bool buttonInterrupt;
int no_bounce_count = 0;
bool interrupt_disabled = false;
int previous_button_value = LOW;

void IRAM_ATTR ISR() {
    // set interrupt flag and then exit
    // disable interrupts
    // check for stable value
    // do something
    // then reenable interrupt

    Serial.println("Setting button interrupt");
    buttonInterrupt = true;
}

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

double count_bounce(int buttonRead ) {
    Serial.printf("counting bounce: %d \r\n", no_bounce_count);
    if (buttonRead != previous_button_value) {
        no_bounce_count = 0; 
    } else {
        no_bounce_count++;
    }
}

void setup_batteryADC() {

}

void setup() {
    Serial.begin(115200);
    motorSetup();
    bluetooth_init();
    adc_setup();
    // buttonSetup();

    pinMode( TUNING_BUTTON_PIN, INPUT_PULLDOWN );
    attachInterrupt( digitalPinToInterrupt(STRING_SWITCH_PIN), ISR, CHANGE );

    sample_freq = 0;
    delay(3000);
    Serial.println("setup complete");
}

void loop() {
	// Serial_Monitor();
    int isTuningOn = digitalRead( TUNING_BUTTON_PIN );
    
    if( buttonInterrupt ){
        Serial.println("Found interrupt");
        // if (!interrupt_disabled){
        noInterrupts();
        //     interrupt_disabled = true;
        // }
        int buttonRead = digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN));

        count_bounce(buttonRead);

        if (no_bounce_count > STABLE_THRESHOLD){
            if (buttonRead == HIGH) {
                Serial.println("Button pressed");
            } else if (buttonRead == LOW) {
                Serial.println("Button released");
            }

            buttonInterrupt = false;
            // interrupt_disabled = false;
            interrupts();
        }

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
