/*
*   The Version of OtterTuner presented at the end of ECE498A on July 26, 2023
*
*   Author: Ryan Bui
*   Additional team members: Cole Gravelle, Jameson Smith, Wenrui Zhang, Alicia Ng
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <math.h>
#include <string>
#include <Preferences.h>

/*
* Freq globals
*/
#define LENGTH 512
#define TARGET_FREQ 82.4

const float sample_freq = 16667;

short rawData[LENGTH];
int len = LENGTH;

int count = 0;
int thresh = 0;
double desired_freq = TARGET_FREQ;
double freq_thres = 0.20 * desired_freq;

/*
* String parsing globals
*/

double tunings[6] = {0};

/*
* BLE globals
*/

// --------
// Constants
// --------
#define SERVICE_UUID        "25AE1441-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_READ_UUID      "25AE1442-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_WRITE_UUID     "25AE1443-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_INDICATE_UUID  "25AE1444-05D3-4C5B-8281-93D4E07420CF"
#define DEVICE_NAME         "OtterTuner"

#define CMD_HELP "help"
#define CMD_INFO "info"
#define CMD_SET_READ "setr="
#define CMD_SET_INDICATE "seti="

// --------
// Global variables
// --------
static BLEServer* g_pServer = nullptr;
static BLECharacteristic* g_pCharRead = nullptr;
static BLECharacteristic* g_pCharWrite = nullptr;
static BLECharacteristic* g_pCharIndicate = nullptr;
static bool g_centralConnected = false;
static std::string g_cmdLine;

Preferences preferences;

/*
* Motor globals
*/

float kp = 100;
float ki = 1;
float kd = 10;

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

/*
* 	BLUETOOTH
*/

// Bluetooth event callbacks
class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer) override
    {
        Serial.println("onConnect");
        g_centralConnected = true;
    }

    void onDisconnect(BLEServer* pServer) override
    {
        Serial.println("onDisconnect, will start advertising");
        g_centralConnected = false;
        BLEDevice::startAdvertising();
    }
};

class MyCharPrintingCallbacks: public BLECharacteristicCallbacks
{
public:
    explicit MyCharPrintingCallbacks(const char* name) : m_name(name) {}

private:
    void PrintEvent(const char* event, const char* value)
    {
        Serial.print(event);
        Serial.print("(");
        Serial.print(m_name.c_str());
        Serial.print(")");
        if (value)
        {
            Serial.print(" value='");
            Serial.print(value);
            Serial.print("'");
        }
        Serial.println();
    }

private:
    void onRead(BLECharacteristic* pCharacteristic) override
    {
        PrintEvent("onRead", pCharacteristic->getValue().c_str());
    }

    void onWrite(BLECharacteristic* pCharacteristic) override
    {
        PrintEvent("onWrite", pCharacteristic->getValue().c_str());

        preferences.begin(DEVICE_NAME, false);
        preferences.putString("tuning", pCharacteristic->getValue().c_str());
        String s = preferences.getString("tuning","");
        Serial.println(s);
        preferences.end();
    }

    void onNotify(BLECharacteristic* pCharacteristic) override
    {
        PrintEvent("onNotify", pCharacteristic->getValue().c_str());
    }

    void onStatus(BLECharacteristic* pCharacteristic, Status status, uint32_t code) override
    {
        std::string event("onStatus:");
        switch (status)
        {
        case SUCCESS_INDICATE: event += "SUCCESS_INDICATE"; break;
        case SUCCESS_NOTIFY: event += "SUCCESS_NOTIFY"; break;
        case ERROR_INDICATE_DISABLED: event += "ERROR_INDICATE_DISABLED"; break;
        case ERROR_NOTIFY_DISABLED: event += "ERROR_NOTIFY_DISABLED"; break;
        case ERROR_GATT: event += "ERROR_GATT"; break;
        case ERROR_NO_CLIENT: event += "ERROR_NO_CLIENT"; break;
        case ERROR_INDICATE_TIMEOUT: event += "ERROR_INDICATE_TIMEOUT"; break;
        case ERROR_INDICATE_FAILURE: event += "ERROR_INDICATE_FAILURE"; break;
        }
        event += ":";
        event += String(code).c_str();
        PrintEvent(event.c_str(), nullptr);
    }

private:
    std::string m_name;
};

bool ParseCommand(const std::string& cmdLine, const std::string& commandKeyword, std::string& outputCommandData)
{
    size_t commandPosition = cmdLine.find(commandKeyword);
    if (commandPosition == std::string::npos)
    {
        return false;
    }
    outputCommandData = cmdLine.substr(commandPosition + commandKeyword.length());
    return true;
}

void PrintInfo()
{
    Serial.println("-------------------------------");
    Serial.println("  Service UUID: "SERVICE_UUID);
    Serial.println(g_centralConnected ? "  Central connected" : "  Central not connected");
    Serial.println("  Characteristics:");

    Serial.print("  Readable: value='");
    Serial.print(g_pCharRead->getValue().c_str());
    Serial.println("' UUID="CHAR_READ_UUID);

    Serial.print("  Writeable: value='");
    Serial.print(g_pCharWrite->getValue().c_str());
    Serial.println("' UUID="CHAR_WRITE_UUID);

    Serial.print("  Indication: value='");
    Serial.print(g_pCharIndicate->getValue().c_str());
    Serial.println("' UUID="CHAR_INDICATE_UUID);
    Serial.println("-------------------------------");

    preferences.begin(DEVICE_NAME, false);
    String s = preferences.getString("tuning","");
    Serial.println(s);
    preferences.end();
}

void bluetooth_init(){
	BLEDevice::init(DEVICE_NAME);
    g_pServer = BLEDevice::createServer();
    g_pServer->setCallbacks(new MyServerCallbacks());
    BLEService* pService = g_pServer->createService(SERVICE_UUID);

    // characteristic for read
    {
        uint32_t propertyFlags = BLECharacteristic::PROPERTY_READ;
        BLECharacteristic* pCharRead = pService->createCharacteristic(CHAR_READ_UUID, propertyFlags);
        pCharRead->setCallbacks(new MyCharPrintingCallbacks("CharRead"));
        pCharRead->setValue("");
        g_pCharRead = pCharRead;
    }

    // characteristic for write
    {
        uint32_t propertyFlags = BLECharacteristic::PROPERTY_WRITE;
        BLECharacteristic* pCharWrite = pService->createCharacteristic(CHAR_WRITE_UUID, propertyFlags);
        pCharWrite->setCallbacks(new MyCharPrintingCallbacks("CharWrite"));
        pCharWrite->setValue("");
        g_pCharWrite = pCharWrite;
    }

    // characteristic for indicate
    {
        uint32_t propertyFlags = BLECharacteristic::PROPERTY_INDICATE;
        BLECharacteristic* pCharIndicate = pService->createCharacteristic(CHAR_INDICATE_UUID, propertyFlags);
        pCharIndicate->setCallbacks(new MyCharPrintingCallbacks("CharIndicate"));
        pCharIndicate->addDescriptor(new BLE2902());
        pCharIndicate->setValue("");
        g_pCharIndicate = pCharIndicate;
    }

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    // this fixes iPhone connection issue (don't know how it works)
    {
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
    }
    BLEDevice::startAdvertising();
}

/*
* 	TUNING SENSING
*/
double measureFrequency() {
	int sum = 0;
  static int sum_old = 0;
	static short pd_state = 0;
	int period = 0;
  double measured_freq;

  for(int attempts = 0; attempts < 32; attempts++){
    for(int i = 0; i < len; i++) {
      sum_old = sum;
      sum = 0;

      for(int k = 0; k < len-i; k++) sum += (rawData[k]-2048)*(rawData[k+i]-2048)/4096;

      if(pd_state == 2 && (sum-sum_old) <= 0){
        period = i;
        // Serial.printf("period: %d\n", period);
        pd_state = 3;
      }

      if(pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0) pd_state = 2;

      if(!i) {
        thresh = sum * 0.5;
        pd_state = 1;
      }
    }

    if(period != 0 && thresh > 70) {
      // offset by 4 because we're bad lol
      measured_freq = sample_freq/period - 4;
      double discrepancy = abs(desired_freq - measured_freq);

      if(discrepancy < freq_thres) {
        Serial.println(measured_freq);
        return measured_freq;
      }
    }
  }
  return -1;
}

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
  double pos = measureFrequency();
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

void parseTuningString(String fullTuning) {
  char delimiter = ',';
  char closing = ']';

  int strLength = fullTuning.length();
  String tuning = "";
  int tuningIndex = 0;

  for(int i = 1; i < strLength; i++) {
    if(fullTuning[i] == delimiter || fullTuning[i] == closing) {
      //skip the comma and the space
      i += 1;
      tunings[tuningIndex] = tuning.toDouble();
      // Serial.printf("tuning at %d: %f\r\n", tuningIndex, tunings[tuningIndex]);
      tuningIndex++;
      tuning = "";
      continue;
    }

    tuning += fullTuning[i];
  }
}

/*
*	MAIN
*/
void setup() {
	Serial.begin(115200);
	motorSetup();
	bluetooth_init();
	delay(3000);
	Serial.println("setup complete");
}

char in;
int mode = 0;

void loop () {
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

	if(count < LENGTH) {
		rawData[count] = analogRead(A0);
		count++;
	} else {
    double prev_freq = desired_freq;

    preferences.begin(DEVICE_NAME, true);
    String tuning = preferences.getString("tuning", "");
    parseTuningString(tuning);

    desired_freq = tunings[0];
    // Serial.printf("desired freq: %f\r\n", desired_freq);
    preferences.end();

		pid();
		count = 0;
	}
}
