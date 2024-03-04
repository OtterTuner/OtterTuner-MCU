#define PWM_FREQUENCY   3000
#define PWM_MAX_VALUE   255

volatile int posi = 0;
long prevT = 0;
float eprev = 0;
float eintegral = 0;

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
    pinMode(MOTOR_IN1_PIN, OUTPUT);
    pinMode(MOTOR_IN2_PIN, OUTPUT);
}

void unwindString(){
    setMotor(-1, PWM_MAX_VALUE, MOTOR_IN1_PIN, MOTOR_IN2_PIN);
}

void setMotor(int dir, int pwmVal, int in1, int in2){
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


