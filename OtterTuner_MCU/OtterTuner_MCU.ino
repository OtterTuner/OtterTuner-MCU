// #include "Guitar_C5.h"
// #include "C4.h"
// #include "E2.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// ---------
// Constants
// ---------

#define LENGTH 5000

#define SERVICE_UUID        "25AE1441-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_READ_UUID      "25AE1442-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_WRITE_UUID     "25AE1443-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_INDICATE_UUID  "25AE1444-05D3-4C5B-8281-93D4E07420CF"

#define CMD_HELP "help"
#define CMD_INFO "info"
#define CMD_SET_READ "setr="
#define CMD_SET_INDICATE "seti="

// ----------------
// Global Variables
// ----------------
static BLEServer* g_pServer = nullptr;
static BLECharacteristic* g_pCharRead = nullptr;
static BLECharacteristic* g_pCharWrite = nullptr;
static BLECharacteristic* g_pCharIndicate = nullptr;
static bool g_centralConnected = false;
static std::string g_cmdLine;

// Frequency analysis variables
const int sample_freq = SOC_ADC_SAMPLE_FREQ_THRES_HIGH/2;

short rawData[LENGTH];
int len = sizeof(rawData);

int count;
int i, k;
long sum, sum_old;
int thresh = 0;
float measured_freq = 0;
float desired_freq = 82.4;
short pd_state = 0;

// PID controller variables

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

// ----------------
// Helper functions
// ----------------

//
// Bluetooth event callbacks
//

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

//
// Frequency Analysis
//

void measureFrequency() {
	sum = 0;
	pd_state = 0;
	int period = 0;

	for(i = 0; i < len; i++) {
		sum_old = sum;
		sum = 0;

		for(k = 0; k < len-i; k++) sum += (rawData[k]-128)*(rawData[k+i]-128)/256;

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

	if(thresh > 100){
		measured_freq = sample_freq/period;
		Serial.println(measured_freq);
	}
}

//
// PID Controller
//

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

void computePid() {
	// set target position
	int target = desired_freq;
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
	int e = measured_freq - target;

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

// --------------
// Main functions
// --------------

void setup() {
	Serial.begin(115200);

	// Set up motor pins for motor control
	pinMode(motor1Pin1, OUTPUT);
	pinMode(motor1Pin2, OUTPUT);
	pinMode(enable1Pin, OUTPUT);

	pinMode(encoderPin1, INPUT);
	pinMode(encoderPin2, INPUT);

	if(len > LENGTH) {
		len = LENGTH;
	}

	// configure LED PWM functionalitites
	ledcSetup(pwmChannel, freq, resolution);

	// attach the channel to the GPIO to be controlled
	ledcAttachPin(enable1Pin, pwmChannel);

	attachInterrupt(digitalPinToInterrupt(encoderPin1),readEncoder,RISING);

	// analogReference(EXTERNAL);
	// TODO: May need to change this depending on what the ADC pin is on ESP32
	// analogRead(A0);

	count = 0;

	Serial.println("BLE Peripherl setup started...");
	BLEDevice::init("ESP32");
	g_pServer = BLEDevice::createServer();
	g_pServer->setCallbacks(new MyServerCallbacks());
	BLEService* pService = g_pServer->createService(SERVICE_UUID);

	//characteristic for read
	{
        uint32_t propertyFlags = BLECharacteristic::PROPERTY_READ;
        BLECharacteristic* pCharRead = pService->createCharacteristic(CHAR_READ_UUID, propertyFlags);
        pCharRead->setCallbacks(new MyCharPrintingCallbacks("CharRead"));
        pCharRead->setValue("ESP32 for read");
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

    Serial.println("BLE Peripheral setup done, advertising");
    Serial.println("");
    PrintInfo();
    PrintHelp();
}

void loop () {
	if(count < LENGTH) {
		count++;
		rawData[count] = analogRead(A0)>>2;
	} else {
		measureFrequency();
		computePid();
		count = 0;
	}
}
