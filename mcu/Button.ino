#define LONG_PRESS  1000
#define DEBOUNCE_TIME       20 * 1000

int pressed_time = 0;

void buttonSetup() {
    pinMode( STRING_SWITCH_PIN, INPUT_PULLDOWN );
    pinMode( TUNING_BUTTON_PIN, INPUT_PULLDOWN );
    attachInterrupt( digitalPinToInterrupt(STRING_SWITCH_PIN), ISR, CHANGE );
}

void buttonHandler() {
    Serial.println("Button interrupt");
    delayMicroseconds(DEBOUNCE_TIME);
    int buttonRead = digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN));
    Serial.printf("button read: %d\r\n", digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN)));
    // for(int i = 0; i < 10; i++) {
    //     Serial.printf("button read: %d\r\n", digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN)));
    // }
    // do something
    if(buttonRead == HIGH) {
        Serial.println("Button pressed");
        pressed_time = millis();
    } else {
        Serial.println("Button released");
        int release_time = millis();

        if(release_time - pressed_time > LONG_PRESS ) {
            Serial.println("Long Press");
        } else {
            Serial.println("Short press");
        }
    }

    // if (time - last_interrupt_time > DEBOUNCE_TIME) {
    //     Serial.println("Button change");
    //     int buttonRead = digitalRead(digitalPinToInterrupt(STRING_SWITCH_PIN));
    //     if( buttonRead == HIGH ) {
    //         // pressed_time = time;
    //         Serial.println("Button pressed");
    //     } else {
    //         Serial.println("Button Released");
    //         if( time - pressed_time < HOLD_THRESHOLD ) {
    //             string_number = ( string_number + 1 ) % 6;
    //             Serial.printf("string number: %d\r\n", string_number);

    //             desired_freq = tunings[string_number];
    //         } else {
    //             Serial.println("Entering Bluetooth mode");
    //             g_centralConnected = false;
    //             BLEDevice::startAdvertising();
    //         }
    //     }

    //     last_interrupt_time = time;
    // }
}
