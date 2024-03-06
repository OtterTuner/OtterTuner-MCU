#include <math.h>
#include <string>
#include <Preferences.h>
#include <driver/adc.h>

#define DEVICE_NAME "OtterTuner"

#define LENGTH 2000
#define MAX_BATTERY_ADC     1300
#define MAX_BATTERY_VOLT    4.2
#define LOW_BATTERY_VOLT    3.5
#define LOW_BATTERY_PERC    LOW_BATTERY_VOLT / MAX_BATTERY_VOLT
#define LOW_BATTERY_ADC     LOW_BATTERY_PERC * MAX_BATTERY_ADC

#define STRING_SWITCH_PIN   2
#define TUNING_BUTTON_PIN   3
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

bool enableSerial = 0;

int LEDS[NUM_LEDS] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN, LED6_PIN};

TaskHandle_t producerTask;
SemaphoreHandle_t semaphore;
volatile bool buttonInterrupt = false;

/*
 * Indices for string numbers are as follows:
 * E2 A2 D3 G3 B3 E4
 * 0  1  2  3  4  5
 */

double tunings[NUM_STRINGS] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};
int string_number = 0;

Preferences preferences;
double desired_freq = tunings[string_number];
double current_frequency = -1;
double sample_freq;
short rawData[LENGTH];
bool bufferFull = false;

void producerTaskCode( void * parameters ) {
    for(;;)
    {
        if (semaphore != NULL && xSemaphoreTake( semaphore, (TickType_t) 10) == pdTRUE )
        {
            int cid = xPortGetCoreID();
            getSamples();
            current_frequency = measureFrequency(sample_freq);
            Serial.printf("current_frequency: %f, desired freq: %f\r\n", current_frequency, desired_freq);
            xSemaphoreGive( semaphore );
            delay(10);
        }
    }
}

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
    desired_freq = get_tuning();
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
    desired_freq = tunings[string_number];
    
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
            if( semaphore != NULL && xSemaphoreTake(semaphore, (TickType_t) 10) == pdTRUE) {
                // Serial.printf("In tuning mode. Current Frequency: %f\r\n", current_frequency);
                pid(current_frequency);
                xSemaphoreGive(semaphore);
            }
        }
    }
    else
    {
        stopMotor();
        float batteryVoltage = analogRead(VBAT_PIN);
        // Serial.printf("battery voltage: %f, low battery threshold: %f, low battery percentage: %f\r\n", batteryVoltage, LOW_BATTERY_ADC, LOW_BATTERY_PERC);

        if( batteryVoltage <= LOW_BATTERY_ADC)
        {
            // Serial.println("Battery low!");
            // LowBatteryAnimation();
        }
    }
}
