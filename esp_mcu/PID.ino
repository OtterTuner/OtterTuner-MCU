// PID Settings
float kp = 100;
float ki = 1;
float kd = 10;

char in;
int mode = 0;

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
  // int target = 250*sin(prevT/1e6);

  // PID constants

  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  // Read the position
  double pos = frequency;
  if(pos == -1){
    setMotor(0,0,enable1Pin,motor1Pin1,motor1Pin2);
    return;
  }

  // error
  double e = pos - target;

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

