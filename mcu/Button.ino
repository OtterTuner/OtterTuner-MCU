#define BUTTON_PIN          7
#define DEBOUNCE_TIME       100
#define MIN_PRESS_TIME      200
#define HOLD_THRESHOLD      1000

volatile int pressed_time = 0;
volatile int last_interrupt_time = 0;

void IRAM_ATTR ISR_Button() {
    int time = millis();
    // Serial.printf("time: %d, last_interrupt_time: %d\r\n", time, last_interrupt_time);

    if (time - last_interrupt_time > DEBOUNCE_TIME) {
        Serial.println("Button change");
        int buttonRead = digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN));
        if( buttonRead == HIGH ) {
            pressed_time = time;
            Serial.println("Button pressed");
        } else {
            Serial.println("Button Released");
            if( time - pressed_time < HOLD_THRESHOLD && time - pressed_time > MIN_PRESS_TIME ) {
                string_number = ( string_number + 1 ) % 6;
                Serial.printf("string number: %d\r\n", string_number);

                desired_freq = tunings[string_number];
            } else if( time - pressed_time > MIN_PRESS_TIME ) {
                Serial.println("Entering Bluetooth mode");
                g_centralConnected = false;
                BLEDevice::startAdvertising();
            }
        }

        last_interrupt_time = time;
    }
}

void IRAM_ATTR ISR2() {
    int buttonRead = digitalRead(digitalPinToInterrupt(BUTTON_PIN));

    if(buttonRead == HIGH) {
        Serial.println("Button pressed");
    } else {
        Serial.println("Button Released");
    }
}

void buttonSetup() {
    pinMode( STRING_SWITCH_PIN, INPUT_PULLDOWN );
    pinMode( TUNING_BUTTON_PIN, INPUT_PULLDOWN );
    attachInterrupt( digitalPinToInterrupt(STRING_SWITCH_PIN), ISR_Button, CHANGE );
    attachInterrupt( digitalPinToInterrupt(BUTTON_PIN), ISR2, CHANGE);
}
