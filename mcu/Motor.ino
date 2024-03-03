#define PWM_FREQUENCY   30000
#define PWM_MAX_VALUE   255
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

void softwarePWM(int pin, int dutyCycle) {
    int onTime = map(dutyCycle, 0, PWM_MAX_VALUE, 0, 100) * PWM_FREQUENCY / 100;
    int offTime = PWM_FREQUENCY - onTime;

    digitalWrite(pin, HIGH);
    delay(onTime);

    digitalWrite(pin, LOW);
    delay(offTime);
}

void motorSetup() {
    // sets the pins as outputs:
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(enable1Pin, OUTPUT);
}

void unwindString(){
    setMotor(-1, PWM_MAX_VALUE, NULL, motor1Pin1, motor1Pin2);
}

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2){
    if(dir == 1){
        softwarePWM(in1, pwmVal);
        digitalWrite(in2,LOW);
    }
    else if(dir == -1){
        digitalWrite(in1,LOW);
        softwarePWM(in2, pwmVal);
    }
    else{
        digitalWrite(in1,LOW);
        digitalWrite(in2,LOW);
    }
}


