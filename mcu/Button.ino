#define BUTTON_PIN      12
#define DEBOUNCE_TIME   100

int last_interrupt_time = millis();

void IRAM_ATTR ISR() {
    Serial.println("Button pressed");

    int interrupt_time = millis();
    if ( interrupt_time - last_interrupt_time > DEBOUNCE_TIME ) {
        string_number = ( string_number + 1 ) % 6;
        Serial.printf("string number: %d\r\n", string_number);

        desired_freq = tunings[string_number];
        last_interrupt_time = interrupt_time;
    }
}

void buttonSetup() {
    pinMode( BUTTON_PIN, INPUT );
    attachInterrupt( BUTTON_PIN, ISR, RISING );
}
