void LED_Setup(){
    for( int i = 0; i < NUM_LEDS; i++ ) {
        pinMode( LEDS[i], OUTPUT );
    }

    digitalWrite(LEDS[string_number], HIGH);
}

void setLEDs(uint8_t mask){
    for(int i = 0; i < NUM_STRINGS; i++){
        if( mask & 0x1 == 1 ) {
            digitalWrite(LEDS[i], HIGH);
        } else {
            digitalWrite(LEDS[i], LOW);
        }
        mask >>= 1;
    }
}

void BLEAnimation() {
    digitalWrite(LEDS[0], HIGH);

    for(int i = 1; i < NUM_LEDS; i++) {
        digitalWrite(LEDS[i], HIGH);
        delay(50);
        digitalWrite(LEDS[i-1], LOW);
        delay(50);
    }

    digitalWrite(LEDS[NUM_LEDS-1], LOW);

    delay(500);

    digitalWrite(LEDS[string_number], HIGH);
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

    digitalWrite(LEDS[string_number], HIGH);
}

void TuningAnimation(double current_freq, double desired_freq) {
    if ( current_freq == -1 ) {
        setLEDs(0);
        return;
    }

    double error = current_freq - desired_freq;
    bool isFlat = current_freq - desired_freq < 0 ? true : false;
    double error_percentage = abs(error) / desired_freq;

    if ( error_percentage <= 0.02 ){
        setLEDs(12);
    } else if( isFlat ) {
        if( error_percentage < 0.10 ) {
            setLEDs(2);
        } else {
            setLEDs(1);
        }
    } else if( !isFlat ){
        if( error_percentage < 0.10 ) {
            setLEDs(16);
        } else {
            setLEDs(32);
        }
    }
}
