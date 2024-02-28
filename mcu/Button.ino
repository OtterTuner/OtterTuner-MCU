#define BUTTON_PIN 12

void IRAM_ATTR ISR() {
    Serial.println("Button pressed");

    string_number = (string_number + 1) % 6;
    desired_freq = tunings[string_number];
    Serial.printf("string number: %d\r\n", string_number);
}

void buttonSetup() {
    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(BUTTON_PIN, ISR, RISING);
}
