# 1 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino"
# 2 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino" 2
# 3 "/Users/workryan/Workspace/OtterTuner-MCU/OtterTuner_MCU/OtterTuner_MCU.ino" 2



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
   digitalWrite(in1,0x1);
   digitalWrite(in2,0x0);
 }
 else if(dir == -1){
   digitalWrite(in1,0x0);
   digitalWrite(in2,0x1);
 }
 else{
   digitalWrite(in1,0x0);
   digitalWrite(in2,0x0);
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
 pinMode(motor1Pin1, 0x03);
 pinMode(motor1Pin2, 0x03);
 pinMode(enable1Pin, 0x03);

 pinMode(encoderPin1, 0x01);
 pinMode(encoderPin2, 0x01);

 // configure LED PWM functionalitites
 ledcSetup(pwmChannel, freq, resolution);

 // attach the channel to the GPIO to be controlled
 ledcAttachPin(enable1Pin, pwmChannel);

 attachInterrupt((((encoderPin1)<48)?(encoderPin1):-1),readEncoder,0x01);

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
 do { __extension__({ unsigned __tmp; __asm__ __volatile__( "rsil	%0, " "3 /* level masked by PS.EXCM */" "\n" : "=a" (__tmp) : : "memory" ); __tmp;}); ; } while (0); // disable interrupts temporarily while reading
 pos = posi;
 do { ; __extension__({ unsigned __tmp; __asm__ __volatile__( "rsil	%0, " "0" "\n" : "=a" (__tmp) : : "memory" ); __tmp;}); } while (0); // turn interrupts back on

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
