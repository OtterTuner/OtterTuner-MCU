#define PWM_FREQUENCY   30000
#define PWM_RESOLUTION  8
#define PWM_MAX_VALUE   255

#define LEDC_CHANNEL_0  0
#define LEDC_CHANNEL_1  1

/*
*	MOTOR CONTROL
*/
void motorSetup() {
    ledcSetup(LEDC_CHANNEL_0, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(LEDC_CHANNEL_1, PWM_FREQUENCY, PWM_RESOLUTION);

    // sets the pins as outputs:
    ledcAttachPin(MOTOR_IN1_PIN, LEDC_CHANNEL_0);
    ledcAttachPin(MOTOR_IN2_PIN, LEDC_CHANNEL_1);
}

void unwindString(){
    setMotor(1, PWM_MAX_VALUE, MOTOR_IN1_PIN, MOTOR_IN2_PIN);
}

void stopMotor(){
    setMotor(0, 0, MOTOR_IN1_PIN, MOTOR_IN2_PIN);
}

void setMotor(int dir, int pwmVal, int in1, int in2){
    if(dir == 1){
        ledcWrite(LEDC_CHANNEL_0, pwmVal);
        ledcWrite(LEDC_CHANNEL_1, 0);
    }
    else if(dir == -1){
        ledcWrite(LEDC_CHANNEL_0, 0);
        ledcWrite(LEDC_CHANNEL_1, pwmVal);
    }
    else{
        ledcWrite(LEDC_CHANNEL_0, 0);
        ledcWrite(LEDC_CHANNEL_1, 0);
    }
}


