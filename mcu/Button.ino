#define STRING_SWITCH_PIN   9
#define DEBOUNCE_TIME       200
#define HOLD_THRESHOLD      1000

volatile int pressed_time = 0;
volatile int last_interrupt_time = 0;

void IRAM_ATTR ISR() {
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
            if( time - pressed_time < HOLD_THRESHOLD ) {
                string_number = ( string_number + 1 ) % 6;
                Serial.printf("string number: %d\r\n", string_number);
            } else {
                Serial.println("Entering Bluetooth mode");
                g_centralConnected = false;
                BLEDevice::startAdvertising();
            }
        }

        last_interrupt_time = time;
    }
}

void buttonSetup() {
    pinMode( STRING_SWITCH_PIN, INPUT_PULLDOWN );
    pinMode( TUNING_BUTTON_PIN, INPUT_PULLDOWN );
    attachInterrupt( digitalPinToInterrupt(STRING_SWITCH_PIN), ISR, CHANGE );
}
