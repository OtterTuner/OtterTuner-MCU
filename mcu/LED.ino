void LED_Setup(){
    for( int i = 0; i < NUM_LEDS; i++ ) {
        pinMode( LEDS[i], OUTPUT );
    }
}

void BLEAnimation() {
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

void LowBatteryAnimation() {
    for(int i = 0; i < NUM_LEDS; i++) {
        digitalWrite(LEDS[i], HIGH);
    }
    delay(500);
    for(int i = 0; i < NUM_LEDS; i++) {
        digitalWrite(LEDS[i], LOW);
    }
    delay(500);
}
