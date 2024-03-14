// PID Settings
float kp = 100;
float ki = 1;
float kd = 10;

char in;
int mode = 0;

volatile int posi = 0;
long prevT = 0;
float eprev = 0;
float eintegral = 0;

void Serial_Monitor() {
    if (Serial.available() > 0) {
        // read the incoming byte:
        in = Serial.read();
        if(in == 'p'){
            mode = 1;
            Serial.println("MODE P");
        } else if(in == 'i'){
            mode = 2;
            Serial.println("MODE I");
        } else if(in == 'd'){
            mode = 3;
            Serial.println("MODE D");
        } else if(in == 'c') {
            Serial.printf("p: %f\r\ni: %f\r\nd: %f\r\n", kp, ki, kd);
        } else if(in == 'n'){
            string_number = (string_number + 1) % 7;
            Serial.printf("New string number: %d\r\n", string_number);
        } else {
            if(in == '+'){
                if(mode == 1){
                    kp++;
                    Serial.printf("p: %f\r\n", kp);
                } else if(mode == 2){
                    ki++;
                    Serial.printf("i: %f\r\n", ki);
                } else if(mode == 3){
                    kd++;
                    Serial.printf("d: %f\r\n", kd);
                }
            } else if(in == '-'){
                if(mode == 1){
                    kp--;
                    Serial.printf("p: %f\r\n", kp);
                } else if(mode == 2){
                    ki--;
                    Serial.printf("i: %f\r\n", ki);
                } else if(mode == 3){
                    kd--;
                    Serial.printf("d: %f\r\n", kd);
                }
            } else {
                mode = 0;
                Serial.println("MODE CLEARED");
            }
        }
    }
}

void pid(double frequency) {
    // set target position
    double target = desired_freq;

    // time difference
    long currT = micros();
    float deltaT = ((float) (currT - prevT))/( 1.0e6 );
    prevT = currT;

    // Read the position
    double pos = frequency;

    // error
    double e = pos - target;
    double error_percent = abs(e/target);

    // Serial.printf("error percentage: %f\r\n", error_percent);

    if(pos == -1 || error_percent <= 0.02){
        setMotor(0, 0, MOTOR_IN1_PIN, MOTOR_IN2_PIN);
        return;
    }

    // derivative
    float dedt = (e-eprev)/(deltaT);

    // integral
    eintegral = eintegral + e*deltaT;

    // control signal
    float u = kp*e + kd*dedt + ki*eintegral;

    // motor power
    float pwr = fabs(u) + 135;
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
    setMotor(dir, pwr, MOTOR_IN1_PIN, MOTOR_IN2_PIN);

    // store previous error
    eprev = e;
}

