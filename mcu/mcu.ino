#include <math.h>
#include <string>
#include <Preferences.h>
#include <driver/adc.h>

#define DEVICE_NAME "OtterTuner"

#define LENGTH 4000
#define MAX_ADC_VALUE       4096

#define STRING_SWITCH_PIN   3
#define TUNING_BUTTON_PIN   4
#define VBAT_PIN            10

#define MOTOR_IN1_PIN   7
#define MOTOR_IN2_PIN   6

#define NUM_STRINGS         6
#define UNWIND_MODE         6

#define NUM_LEDS        6
#define LED1_PIN        35
#define LED2_PIN        37
#define LED3_PIN        36
#define LED4_PIN        34
#define LED5_PIN        9
#define LED6_PIN        8

int LEDS[NUM_LEDS] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN, LED6_PIN};

TaskHandle_t producerTask;
SemaphoreHandle_t semaphore;

Preferences preferences;
double desired_freq;
double current_frequency;
double sample_freq;
short rawData[LENGTH];
bool bufferFull = false;

void producerTaskCode( void * parameters ) {
    for(;;)
    {
        if (semaphore != NULL && xSemaphoreTake( semaphore, (TickType_t) 10) == pdTRUE )
        {
            int cid = xPortGetCoreID();
            // Serial.printf("Producer task code running on core %d\r\n", cid);
            getSamples();
            current_frequency = measureFrequency(sample_freq);
            Serial.printf("current_frequency: %f, desired freq: %f\r\n", current_frequency, desired_freq);
            xSemaphoreGive( semaphore );
            delay(10);
        }
    }
}

/*
 * Indices for string numbers are as follows:
 * E2 A2 D3 G3 B3 E4
 * 0  1  2  3  4  5
 */

double tunings[NUM_STRINGS] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};
int string_number = 0;

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

void setup() {
    Serial.begin(115200);
    motorSetup();
    bluetooth_init();
    adc_setup();
    buttonSetup();
    LED_Setup();
    sample_freq = 0;
    pinMode( VBAT_PIN, OUTPUT );

    semaphore = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(
        producerTaskCode,
        "Producer Task",
        10000,
        NULL,
        1,
        &producerTask,
        0
    );

	Serial.println("setup complete");
}

void loop() {
	// Serial_Monitor();
    int isTuningOn = digitalRead( TUNING_BUTTON_PIN );
    
    if( buttonInterrupt )
    {
        buttonHandler();
        buttonInterrupt = false;
    }
    else if ( isTuningOn == HIGH )
    {

        // If we're at string number 6, then we're unwinding the string
        if (string_number == UNWIND_MODE)
        {
            Serial.println("Unwinding string");
            unwindString();
        }
        else
        {
            double prev_freq = desired_freq;
            desired_freq = get_tuning();
            if( semaphore != NULL && xSemaphoreTake(semaphore, (TickType_t) 10) == pdTRUE) {
                Serial.printf("In tuning mode. Current Frequency: %f\r\n", current_frequency);
                pid(current_frequency);
                xSemaphoreGive(semaphore);
            }
        }
    }
    else
    {
        // TODO: Insert battery reading code
        float batteryVoltage = analogRead(ADC1_CHANNEL_MAX);
        float batteryPercentage = batteryVoltage / MAX_ADC_VALUE;

        if( batteryPercentage <= 0.2)
        {
            Serial.println("Battery low!");
            LowBatteryAnimation();
        }
    }
}
