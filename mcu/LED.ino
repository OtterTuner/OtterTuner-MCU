void LED_Setup(){
    for( int i = 0; i < NUM_LEDS; i++ ) {
        pinMode( LEDS[i], OUTPUT );
    }
}

void BLEAnimation() {
    digitalWrite(LEDS[0], HIGH);

    for(int i = 1; i < NUM_LEDS; i++) {
        digitalWrite(LEDS[i], HIGH);
        delay(10);
        digitalWrite(LEDS[i-1], LOW);
        delay(10);
    }

    digitalWrite(LEDS[NUM_LEDS-1], LOW);
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
