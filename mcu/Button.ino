#define LONG_PRESS      1000
#define DEBOUNCE_TIME   20 * 1000
#define NUM_LEDS        6
#define LED1_PIN        35
#define LED2_PIN        37
#define LED3_PIN        36
#define LED4_PIN        34
#define LED5_PIN        9
#define LED6_PIN        8

int pressed_time = 0;

int LEDS[NUM_LEDS] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN, LED6_PIN};

void buttonSetup() {
    pinMode( STRING_SWITCH_PIN, INPUT_PULLDOWN );
    pinMode( TUNING_BUTTON_PIN, INPUT_PULLDOWN );

    // setup LEDs
    for( int i = 0; i < NUM_LEDS; i++ ) {
        pinMode( LEDS[i], OUTPUT );
    }

    attachInterrupt( digitalPinToInterrupt(STRING_SWITCH_PIN), ISR, CHANGE );
}

void ledPattern() {
    digitalWrite(LEDS[0], HIGH);
    Serial.printf("LED[%d] value: %d\r\n", 0, digitalRead(LEDS[0]));

    for(int i = 1; i < NUM_LEDS; i++) {
        digitalWrite(LEDS[i], HIGH);
        Serial.printf("LED[%d] value: %d\r\n", i, digitalRead(LEDS[i]));
        delay(10);
        digitalWrite(LEDS[i-1], LOW);
        Serial.printf("LED[%d] value: %d\r\n", i-1, digitalRead(LEDS[i-1]));
        delay(10);
    }

    digitalWrite(LEDS[NUM_LEDS-1], LOW);
    Serial.printf("LED[%d] value: %d\r\n", NUM_LEDS-1, digitalRead(LEDS[NUM_LEDS-1]));
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
            ledPattern();
            ledPattern();
        } else {
            string_number = (string_number + 1) % 7;
            if(string_number == 6) {
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
