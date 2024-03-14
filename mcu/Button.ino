#define LONG_PRESS      1000
#define DEBOUNCE_TIME   20 * 1000

int pressed_time = 0;

void buttonSetup() {
    pinMode( STRING_SWITCH_PIN, INPUT_PULLDOWN );
    pinMode( TUNING_BUTTON_PIN, INPUT_PULLDOWN );

    attachInterrupt( digitalPinToInterrupt(STRING_SWITCH_PIN), ISR, CHANGE );
}

void buttonHandler() {
    delayMicroseconds(DEBOUNCE_TIME);
    int buttonRead = digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN));

    if(buttonRead == HIGH) {
        pressed_time = millis();
    } else {
        int release_time = millis();

        if(release_time - pressed_time > LONG_PRESS ) {
            Serial.println("Entering bluetooth pairing mode");
            g_centralConnected = false;
            BLEDevice::startAdvertising();
            BLEAnimation();
        } else {
            string_number = (string_number + 1) % 7;
            if(string_number == UNWIND_MODE) {
                Serial.println("Unwinding string mode");
                digitalWrite(LEDS[NUM_LEDS-1], LOW);
            } else if (string_number > 0){
                Serial.printf("New String number: %d\r\n", string_number);
                digitalWrite(LEDS[string_number-1], LOW);
                digitalWrite(LEDS[string_number], HIGH);
            } else {
                Serial.printf("New String number: %d\r\n", string_number);
                digitalWrite(LEDS[0], HIGH);
            }

        }
    }
}
