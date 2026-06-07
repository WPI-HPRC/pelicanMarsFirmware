#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <SPI.h>

#if defined(MARSv20) || defined(MARSv21)
SPIClass SENSORS_SPI(SENSORS_SPI_MOSI, SENSORS_SPI_MISO, SENSORS_SPI_SCK);
TwoWire GPS_I2C(GPS_I2C_SDA, GPS_I2C_SCL);
HardwareSerial GPS_SERIAL(GPS_SERIAL_RX, GPS_SERIAL_TX);
TwoWire CONNECTOR_I2C(CONNECTOR_I2C_SDA, CONNECTOR_I2C_SCL);
SPIClass CAMERA_SPI(CAMERA_MOSI, CAMERA_MISO, CAMERA_SCK);
HardwareSerial RADIO_SERIAL(RADIO_SERIAL_RX, RADIO_SERIAL_TX);
#define MARS
#else
#define LED_POLARIS (6)
#define RADIO_SERIAL Serial2
#define RADIO_M0 21
#define RADIO_M1 20
#define RADIO_AUX 24
#define POLARIS
#endif

#include "LoRaE22.h"
#include "RadioConfigs.h"

// #define RADIO_DEBUG

const char* callsign = "KV0R";
LoRaE22 radioModule(&RADIO_SERIAL, RADIO_M0, RADIO_M1, RADIO_AUX, callsign);

// Implement this per your platform, then pass a callback to it in setup().
// This is a MARS SPECIFIC implementation.
bool changeSerialPortConfig(RadioConfigTypes::SerialSpeeds baudRate, RadioConfigTypes::ParityConfig parity){
  // this is safe to call even when the port is not open.
  RADIO_SERIAL.end();

  uint32_t baud = 0;
  uint16_t parityConfig = 0;
  
  // the radio's baud rates don't follow any pattern over the entire range, so ugly switch statement it is
  switch(baudRate){
    case RadioConfigTypes::SerialSpeeds::BAUD_1200:
      baud = 1200; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_2400:
      baud = 2400; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_4800:
      baud = 4800; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_9600:
      baud = 9600; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_19200:
      baud = 19200; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_38400:
      baud = 38400; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_57600:
      baud = 57600; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_115200:
      baud = 115200; break;
  };
  // this is just easier
  switch(parity){
    case RadioConfigTypes::ParityConfig::Parity_8N1:
      parityConfig = SERIAL_8N1; break;
    case RadioConfigTypes::ParityConfig::Parity_8E1:
      parityConfig = SERIAL_8E1; break;
    case RadioConfigTypes::ParityConfig::Parity_8O1:
      parityConfig = SERIAL_8O1; break;
  };
  
  RADIO_SERIAL.begin(baud, parityConfig);

  return true;
};

void radioInit();
void radioUpdate();

unsigned long blinkTimer;


void setup() {
  SerialUSB.begin(921600); while(!SerialUSB){};
  #ifdef MARS
  pinMode(LED_BLUE, OUTPUT); digitalWrite(LED_BLUE, HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, LOW);
  pinMode(LED_RED, OUTPUT); digitalWrite(LED_RED, LOW);
  #else
  pinMode(LED_POLARIS, OUTPUT); digitalWrite(LED_POLARIS, LOW);
  pinMode(LED_BUILTIN, OUTPUT); digitalWrite(LED_BUILTIN, LOW);
  #endif

  radioInit();
  blinkTimer = millis();

}

void loop() {
  if(RADIO_SERIAL.available() > 0){
    SerialUSB.write(RADIO_SERIAL.read());
    #ifdef MARS
    digitalToggle(LED_GREEN);
    #else
    digitalToggle(LED_POLARIS);
    #endif
  }
  if(SerialUSB.available() > 0){
    RADIO_SERIAL.write(SerialUSB.read());
    #ifdef MARS
    digitalToggle(LED_GREEN);
    #else
    digitalToggle(LED_POLARIS);
    #endif
  }
  if(millis() - blinkTimer > 500){
    #ifdef MARS
    digitalToggle(LED_BLUE);
    #else
    digitalToggle(LED_BUILTIN);
    #endif
    blinkTimer = millis();
  }
}

void radioInit(){
  // build our config
  RadioConfig config;
  config.address = ADDRESS;
  config.networkId = NETWORKID;
  config.encryptionKey = ENCRYPTIONKEY;
  config.parityConfig = PARITYCONFIG;
  config.serialSpeed = SERIALSPEED;
  config.airDataRate = AIRDATARATE;
  config.packetSize = PACKETSIZE;
  config.worMode = WORMODE;
  config.worPeriod = WORPERIOD;
  config.relayMode = RELAYMODE;
  config.destination = DESTINATIONMODE;
  config.txPower = dBm33;
  config.ambientRSSIEnabled = AMBIENTRSSI;
  config.rssiReadingsEnabled = RSSIREADINGS;
  config.listenBeforeTxEnable = LISTENBEFORETX;
  radioModule.setConfig(config);
  radioModule.setFrequency(FREQUENCY);

  radioModule.changeSerialPortCallback(changeSerialPortConfig);
  radioModule.setTimeout(2000);


  #ifdef RADIO_DEBUG
    uint8_t configBuffer[9];
    radioModule.buildConfigBuffer((unsigned char*)&configBuffer);
    SerialUSB.print("desired config registers: ");
    for (size_t i; i<sizeof(configBuffer);i++){
      SerialUSB.printf("%0X ",configBuffer[i]);
    }
    SerialUSB.println("");
  #endif

  int8_t code = radioModule.init(3);
  radioModule.setMode(RadioMode::Normal);
  #ifdef MARS
  if(code < 0){digitalWrite(LED_RED, HIGH);}
  else{digitalWrite(LED_GREEN, HIGH);}
  #else
  if(code < 0){digitalWrite(LED_POLARIS, HIGH);}
  else{digitalWrite(LED_BUILTIN, HIGH);}

  #endif

  #ifdef RADIO_DEBUG
    SerialUSB.println("done initing");
    SerialUSB.println(code);
  #endif

  
}
