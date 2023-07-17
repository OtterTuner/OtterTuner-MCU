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

void setup() {
  // sets the pins as outputs:
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
}

void loop() {

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