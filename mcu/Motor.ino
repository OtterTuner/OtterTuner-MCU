/*
* Motor globals
*/

// Motor 1
int motor1Pin1 = 7;
int motor1Pin2 = 6;
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

/*
*	MOTOR CONTROL
*/

void motorSetup() {
  // sets the pins as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
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


