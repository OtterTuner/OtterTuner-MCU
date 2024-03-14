#include <math.h>
#include <driver/adc.h>

#define LENGTH 4000

double tunings[6] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};
int string_number=1;
double desired_freq;
double sample_freq;
int count = 0;
short rawData[LENGTH];
int len = LENGTH;
int thresh = 0;
double freq_thres = 0.20 * desired_freq;
int motor1Pin1 = 6;
int motor1Pin2 = 7;
float kp = 100;
float ki = 1;
float kd = 10;
volatile int posi = 0;
long prevT = 0;
float eprev = 0;
float eintegral = 0;
const int freq = 30000;

void setMotor(int dir, int pwmVal, int in1, int in2){
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

void pid() {
  // set target position
  double target = desired_freq;
  // int target = 250*sin(prevT/1e6);

  // PID constants

  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  // Read the position
  double pos = measureFrequency(sample_freq);
  Serial.print("Current freq: ");
  Serial.println(pos);
  if(pos == -1){
    setMotor(0,0,motor1Pin1,motor1Pin2);
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
  Serial.print("Current dir: ");
  Serial.println(dir);
  setMotor(dir,pwr,motor1Pin1,motor1Pin2);

  // store previous error
  eprev = e;
}

double measureFrequency(double sample_freq) {
    int sum = 0;
    static int sum_old = 0;
    static short pd_state = 0;
    int period = 0;
    double measured_freq;

    for(int i = 0; i < len; i++) {
        sum_old = sum;
        sum = 0;

        for(int k = 0; k < len-i; k++) sum += (rawData[k]-2048)*(rawData[k+i]-2048)/4096;

        if(pd_state == 2 && (sum-sum_old) <= 0){
            period = i;
            pd_state = 3;
        }

        if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

        if(!i) {
            thresh = sum * 0.5;
            pd_state = 1;
        }
    }

    if(period != 0) {
        measured_freq = sample_freq/period;
        freq_thres = 0.20 * desired_freq;

        double discrepancy = abs(desired_freq - measured_freq);

        if(discrepancy < freq_thres) {
            return measured_freq;
        }
    }

    return -1;
}

void motorSetup() {
  // sets the pins as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
}

void adc_setup(){
    adc_digi_init_config_t config;
	config.max_store_buf_size = 1024;
	config.adc1_chan_mask = BIT(6);
	config.adc2_chan_mask = 0;
	config.conv_num_each_intr = 256;

	adc_digi_pattern_config_t adc_pattern;
	adc_pattern.atten = ADC_ATTEN_DB_0;
	adc_pattern.channel = ADC1_CHANNEL_6;
	adc_pattern.unit = ADC_UNIT_1;
	adc_pattern.bit_width = 12;

  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0);

	adc_digi_configuration_t controller_config;
	controller_config.conv_limit_en = 0;
	controller_config.conv_limit_num = 250;
	controller_config.pattern_num = 1;
	controller_config.adc_pattern = &adc_pattern;
	controller_config.sample_freq_hz = 60000;
	controller_config.conv_mode = ADC_CONV_SINGLE_UNIT_1;
	controller_config.format = ADC_DIGI_OUTPUT_FORMAT_TYPE2;
}

void setup() {
	Serial.begin(115200);
  adc_setup();
  motorSetup();
  sample_freq = 0;
	Serial.println("setup complete");
}

void loop() {
  int in = Serial.read();
  if (in == 'n') {
      string_number = (string_number + 1) % 6;
      desired_freq = tunings[string_number];
      Serial.printf("string number: %d\r\n", string_number);
  } else if (in == 'p') {
      string_number = (string_number - 1) % 6;
      desired_freq = tunings[string_number];
      Serial.printf("string number: %d\r\n", string_number);
  }

  double startTime = millis();
  for (int i = 0; i < LENGTH; i++) {
      rawData[i] = adc1_get_raw(ADC1_CHANNEL_4);
  }
  double endTime = millis();
  sample_freq = (LENGTH / (endTime - startTime)) * 1000;

	pid();

  count = 0;
}
