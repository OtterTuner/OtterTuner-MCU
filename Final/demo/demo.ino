#include <math.h>
#include <string>
#include <Preferences.h>

#define DEVICE_NAME "OtterTuner"
#define LENGTH 512

Preferences preferences;
double tunings[6] = {0};
int string_number = 0;
double desired_freq = tunings[string_number];
int count = 0;
short rawData[LENGTH];

double get_tuning(){
	preferences.begin(DEVICE_NAME, true);
	String tuning = preferences.getString("tuning", "");
	parseTuningString(tuning);
	preferences.end();

	return tunings[string_number];
}

void setup() {
	Serial.begin(115200);
	motorSetup();
	bluetooth_init();
	delay(3000);
	Serial.println("setup complete");
}

void loop() {
	Serial_Monitor();

	if(count < LENGTH) {
		rawData[count] = analogRead(A0);
		count++;
	} else {
		double prev_freq = desired_freq;
		desired_freq = get_tuning();
		// Serial.printf("desired freq: %f\r\n", desired_freq);

		double current_frequency = measureFrequency();
		string_number = stringCheck(current_frequency);

		pid(current_frequency);
		count = 0;
	}
}
