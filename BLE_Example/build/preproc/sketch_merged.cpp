# 1 "/Users/workryan/Documents/Arduino/BLE_Example/BLE_Example.ino"
/*
 * BLEProofPeripheral.cpp
 *
 * Created by Alexander Lavrushko on 22/03/2021.
 *
 * @brief BLEProof Peripheral ESP32
 * Bluetooth Low Energy Peripheral (also called Slave, Server) demo application for ESP32
 * 1. Advertises one service with 3 characteristics:
 *    - characteristic which supports read (BLE Central can only read)
 *    - characteristic which supports write (BLE Central can only write, with response)
 *    - characteristic which supports indication (BLE Central can only subscribe and listen for indications)
 * 2. Provides command line interface for changing values of characteristics:
 *    - use Arduino Serial Monitor with 115200 baud, and option 'Newline' or 'Carriage return' or 'Both'
 */

# 17 "/Users/workryan/Documents/Arduino/BLE_Example/BLE_Example.ino" 2
# 18 "/Users/workryan/Documents/Arduino/BLE_Example/BLE_Example.ino" 2
# 19 "/Users/workryan/Documents/Arduino/BLE_Example/BLE_Example.ino" 2
# 20 "/Users/workryan/Documents/Arduino/BLE_Example/BLE_Example.ino" 2

// --------
// Constants
// --------
# 34 "/Users/workryan/Documents/Arduino/BLE_Example/BLE_Example.ino"
// --------
// Global variables
// --------
static BLEServer* g_pServer = nullptr;
static BLECharacteristic* g_pCharRead = nullptr;
static BLECharacteristic* g_pCharWrite = nullptr;
static BLECharacteristic* g_pCharIndicate = nullptr;
static bool g_centralConnected = false;
static std::string g_cmdLine;

// --------
// Bluetooth event callbacks
// --------
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

// --------
// Application lifecycle: setup & loop
// --------
void setup()
{
    Serial.begin(115200);
 delay(1000);
    Serial.println("BLE Peripheral setup started");

    BLEDevice::init("ESP32");
    g_pServer = BLEDevice::createServer();
    g_pServer->setCallbacks(new MyServerCallbacks());
    BLEService* pService = g_pServer->createService("25AE1441-05D3-4C5B-8281-93D4E07420CF");

    // characteristic for read
    {
        uint32_t propertyFlags = BLECharacteristic::PROPERTY_READ;
        BLECharacteristic* pCharRead = pService->createCharacteristic("25AE1442-05D3-4C5B-8281-93D4E07420CF", propertyFlags);
        pCharRead->setCallbacks(new MyCharPrintingCallbacks("CharRead"));
        pCharRead->setValue("ESP32 for read");
        g_pCharRead = pCharRead;
    }

    // characteristic for write
    {
        uint32_t propertyFlags = BLECharacteristic::PROPERTY_WRITE;
        BLECharacteristic* pCharWrite = pService->createCharacteristic("25AE1443-05D3-4C5B-8281-93D4E07420CF", propertyFlags);
        pCharWrite->setCallbacks(new MyCharPrintingCallbacks("CharWrite"));
        pCharWrite->setValue("");
        g_pCharWrite = pCharWrite;
    }

    // characteristic for indicate
    {
        uint32_t propertyFlags = BLECharacteristic::PROPERTY_INDICATE;
        BLECharacteristic* pCharIndicate = pService->createCharacteristic("25AE1444-05D3-4C5B-8281-93D4E07420CF", propertyFlags);
        pCharIndicate->setCallbacks(new MyCharPrintingCallbacks("CharIndicate"));
        pCharIndicate->addDescriptor(new BLE2902());
        pCharIndicate->setValue("");
        g_pCharIndicate = pCharIndicate;
    }

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("25AE1441-05D3-4C5B-8281-93D4E07420CF");
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

void loop()
{
    if (!Serial.available())
    {
        return;
    }

    char c = Serial.read();
    if (c != '\r' && c != '\n')
    {
        g_cmdLine += c;
        return;
    }

    std::string cmdLine;
    std::swap(g_cmdLine, cmdLine);
    if (cmdLine.empty())
    {
        return;
    }

    std::string commandData;
    if (ParseCommand(cmdLine, "help", commandData))
    {
        PrintHelp();
        return;
    }

    if (ParseCommand(cmdLine, "info", commandData))
    {
        PrintInfo();
        return;
    }

    if (ParseCommand(cmdLine, "setr=", commandData))
    {
        Serial.print("Setting read characteristic: '");
        Serial.print(commandData.c_str());
        Serial.println("'");
        g_pCharRead->setValue(commandData);
        return;
    }

    if (ParseCommand(cmdLine, "seti=", commandData))
    {
        Serial.print("Setting indicate characteristic: '");
        Serial.print(commandData.c_str());
        Serial.println("'");
        g_pCharIndicate->setValue(commandData);
        g_pCharIndicate->indicate();
        return;
    }

    Serial.print("ERROR: command not recognized: '");
    Serial.print(cmdLine.c_str());
    Serial.println("'");
}

// --------
// Helper functions
// --------
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
    Serial.println("  Service UUID: ""25AE1441-05D3-4C5B-8281-93D4E07420CF");
    Serial.println(g_centralConnected ? "  Central connected" : "  Central not connected");
    Serial.println("  Characteristics:");

    Serial.print("  Readable: value='");
    Serial.print(g_pCharRead->getValue().c_str());
    Serial.println("' UUID=""25AE1442-05D3-4C5B-8281-93D4E07420CF");

    Serial.print("  Writeable: value='");
    Serial.print(g_pCharWrite->getValue().c_str());
    Serial.println("' UUID=""25AE1443-05D3-4C5B-8281-93D4E07420CF");

    Serial.print("  Indication: value='");
    Serial.print(g_pCharIndicate->getValue().c_str());
    Serial.println("' UUID=""25AE1444-05D3-4C5B-8281-93D4E07420CF");
    Serial.println("-------------------------------");
}

void PrintHelp()
{
    Serial.println("-------------------------------");
    Serial.println("  Command line interface:");
    Serial.println("  1. ""help"" - print this description of command line interface");
    Serial.println("  2. ""info"" - print current state of BLE Peripheral");
    Serial.println("  3. ""setr=""<value> - set value to readable characteristic");
    Serial.println("       Set 'abc def': ""setr=""abc def");
    Serial.println("       Set empty value: ""setr=");
    Serial.println("  4. ""seti=""<value> - set value to indication characteristic, and send indication to Central");
    Serial.println("       Set 'abc def': ""seti=""abc def");
    Serial.println("       Set empty value: ""seti=");
    Serial.println("-------------------------------");
    Serial.println("Waiting for command line input...");
}
