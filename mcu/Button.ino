#define LONG_PRESS  1000
#define DEBOUNCE_TIME       20 * 1000

int pressed_time = 0;

void buttonSetup() {
    pinMode( STRING_SWITCH_PIN, INPUT_PULLDOWN );
    pinMode( TUNING_BUTTON_PIN, INPUT_PULLDOWN );
    attachInterrupt( digitalPinToInterrupt(STRING_SWITCH_PIN), ISR, CHANGE );
}

void buttonHandler() {
    delayMicroseconds(DEBOUNCE_TIME);
    int buttonRead = digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN));
    Serial.printf("button read: %d\r\n", digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN)));

    if(buttonRead == HIGH) {
        Serial.println("Button pressed");
        pressed_time = millis();
    } else {
        Serial.println("Button released");
        int release_time = millis();

        if(release_time - pressed_time > LONG_PRESS ) {
            Serial.println("Entering bluetooth pairing mode");
            g_centralConnected = false;
            BLEDevice::startAdvertising();
        } else {
            string_number = (string_number + 1) % 6;
            Serial.printf("New String number: %d\r\n", string_number);
        }
    }
}
