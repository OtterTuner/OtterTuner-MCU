#include <Arduino.h>
#line 1 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
#include <math.h>
#include "C4.h"

#define LENGTH 512

float minFreq = 60.0;
const float sample_freq = 22050;

// short rawData[LENGTH];
int len = sizeof(rawData);
int count;
int i, k;
long sum, sum_old;
int thresh = 0;
float freq_per = 0;
short pd_state = 0;

// Motor 1
int motor1Pin1 = 37; 
int motor1Pin2 = 35; 
int enable1Pin = 36; 

// Encoder 1
int encoderPin1 = 6;
int encoderPin2 = 7;

volatile int posi = 0;
long prevT = 0;
float eprev = 0;
float eintegral = 0;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;

#line 37 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
void measureFrequency();
#line 70 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
void setMotor(int dir, int pwmVal, int pwm, int in1, int in2);
#line 87 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
void readEncoder();
#line 97 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
void setup();
#line 123 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
void loop();
#line 37 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
void measureFrequency() {
	sum = 0;
	pd_state = 0;
	int period = 0;
	for(i = 0; i < len; i++) {
		sum_old = sum;
		sum = 0;
		for(k = 0; k < len-i; k++) sum += (rawData[k]-128)*(rawData[k+i]-128)/256;

		// Serial.println(sum);

		if(pd_state == 2 && (sum-sum_old) <= 0){
			period = i;
			pd_state = 3;
		}

		if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

		if(!i) {
			thresh = sum * 0.5;
			pd_state = 1;
		}

		// for(i=0; i < len; i++) Serial.println(rawData[i]);

		if(thresh > 100){
			freq_per = sample_freq/period;
			Serial.println(freq_per);
		}
		count = 0;
	}
}

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2){
	ledcWrite(pwmChannel, pwmVal);   

	if(dir == 1){
	  digitalWrite(in1,HIGH);
	  digitalWrite(in2,LOW);
	}
	else if(dir == -1){
	  digitalWrite(in1,LOW);
	  digitalWrite(in2,HIGH);
	}
	else{
	  digitalWrite(in1,LOW);
	  digitalWrite(in2,LOW);
	}  
}

void readEncoder(){
	int b = digitalRead(encoderPin2);
	if(b > 0){
	  posi++;
	}
	else{
	  posi--;
	}
}

void setup() {
	// analogReference(EXTERNAL);
	// TODO: May need to change this depending on what the ADC pin is on ESP32
	// analogRead(A0);
	pinMode(motor1Pin1, OUTPUT);
	pinMode(motor1Pin2, OUTPUT);
	pinMode(enable1Pin, OUTPUT);

	pinMode(encoderPin1, INPUT);
	pinMode(encoderPin2, INPUT);

	// configure LED PWM functionalitites
	ledcSetup(pwmChannel, freq, resolution);

	// attach the channel to the GPIO to be controlled
	ledcAttachPin(enable1Pin, pwmChannel);

	attachInterrupt(digitalPinToInterrupt(encoderPin1),readEncoder,RISING);

	Serial.begin(115200);
	delay(3000);
	count = 0;

	measureFrequency();
}

void loop () {
	
	// if(count < LENGTH) {
	// 	count++;
	// 	rawData[count] = analogRead(A0)>>2;
	// } else {
	// }

	// set target position
	int target = 1500;
	// int target = 250*sin(prevT/1e6);

	// PID constants
	float kp = 3.0;
	float kd = 0.025;
	float ki = 0.0;

	// time difference
	long currT = micros();
	float deltaT = ((float) (currT - prevT))/( 1.0e6 );
	prevT = currT;

	// Read the position
	int pos = 0; 
	noInterrupts(); // disable interrupts temporarily while reading
	pos = posi;
	interrupts(); // turn interrupts back on

	// error
	int e = pos - target;

	// derivative
	float dedt = (e-eprev)/(deltaT);

	// integral
	eintegral = eintegral + e*deltaT;

	// control signal
	float u = kp*e + kd*dedt + ki*eintegral;

	// motor power
	float pwr = fabs(u) + 130;
	if( pwr > 255 ){
	  pwr = 255;
	}

	if( pwr <= 135 ){
	  pwr = 0;
	}

	// motor direction
	int dir = 1;
	if(u<0){
	  dir = -1;
	}

	// signal the motor
	setMotor(dir,pwr,enable1Pin,motor1Pin1,motor1Pin2);


	// store previous error
	eprev = e;
}

