// @Author jmaumene
// https://www.maumene.fr
//
// Double counter I2C on pin 2 and 3 ( named 0 and 1 ) 
//
// For python script reader or writer see :
// https://github.com/jmaumene/Raspberry-Pi-Code/tree/master/I2C/arduino_counter 

#include <Wire.h>
#include <EEPROM.h>


// Init PulseA
unsigned long pulseACount = 0;
unsigned long pulseATime;
const int pulseAPin = 0;
const int addrStoreA = 0;

// Init PulseB
unsigned long pulseBCount = 0;
unsigned long pulseBTime;
const int pulseBPin = 1;
const int addrStoreB = 4;

// Init I2C
#define SLAVE_ADDRESS 0x12
uint8_t buffer[4];
int bufferIndex = 0;
unsigned long countReceveid;

// CONFIG
unsigned long nowMillis;
const int ledPin = 13; 
int interval = 600;

void setup()
{
  // INIT I2C
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);


  // PIN CONFIG   
  pinMode(ledPin,OUTPUT);
  pinMode(pulseAPin, INPUT);
  pinMode(pulseBPin, INPUT);

  // INTERUPT
  attachInterrupt(pulseAPin, onPulseA, RISING);
  attachInterrupt(pulseBPin, onPulseB, RISING);

  //saveCountA();
  //saveCountB();
  readCountA();
  readCountB();

  // SERIAL
  //Serial.begin(9600);
  //Serial.println("Init v1.0");
  //Serial.print("Address : ");
  //Serial.println(SLAVE_ADDRESS);
  //Serial.print("Cpt A :");
  //Serial.println(pulseACount);
  //Serial.print("Cpt B :");
  //Serial.println(pulseBCount);

}

void loop()
{
  //delay(3000);
  //Serial.println("RESULT");
  //Serial.print("Count A :");
  //Serial.println(pulseACount);
  //Serial.print("Count B : ");
  //Serial.println(pulseBCount);
}


void ledBlink()
{
  digitalWrite(ledPin, HIGH); 
  delay(1000);
  digitalWrite(ledPin, LOW);
}

// COUNTER A
void onPulseA(void)
{
  nowMillis = millis();
  if((nowMillis - pulseATime) < interval) {
    pulseATime = nowMillis;
    return;
  }
  //Serial.println("BIP A");
  ledBlink();
  pulseACount++;
  pulseATime = nowMillis;
  saveCountA();
}

void saveCountA()
{
  saveLong(addrStoreA, pulseACount, 0);
}

void readCountA()
{
  //Serial.print("Read A: ");
  pulseACount = readLong(addrStoreA);
  //Serial.println(pulseACount);
}

// COUNTER B
void onPulseB(void)
{
  nowMillis = millis();
  if((nowMillis - pulseBTime) < interval) {
    pulseBTime = nowMillis;
    return;
  }
  //Serial.println("BIP B");
  ledBlink();
  pulseBCount++;
  pulseBTime = nowMillis;
  saveCountB();
}

void saveCountB()
{
  saveLong(addrStoreB,pulseBCount, 0);
}

void readCountB()
{
  //Serial.print("Read B: ");
  pulseBCount = readLong(addrStoreB);
  //Serial.println(pulseBCount);
}


// FUNCTION I2C
void receiveData(int byteCount)
{
  int cmd     = Wire.read();
  //Serial.print("I2C : ");
  //Serial.print(byteCount);
  //Serial.print(" - cmd : ");
  //Serial.println(cmd);

  // 0x01 = read 1
  if( cmd == 0x01 ) {
    i2cSendCounter(pulseACount);
  } 
  else if ( cmd == 0x02 ) {
    i2cSendCounter(pulseBCount);
  }
  else if( cmd == 0x03 ) {
    pulseACount = i2cSetCounter();
    saveLong(addrStoreA, pulseACount, 1);
  }
  else if( cmd == 0x04 ) {
    pulseBCount = i2cSetCounter();
    saveLong(addrStoreB, pulseBCount, 1);
  } 
  else {
    //Serial.print("unknow command");
    //Serial.println(cmd);
  }
}

void sendData()
{
  //Serial.print("SendData index :");
  //Serial.println(bufferIndex);
  if(bufferIndex <= 3) {
    //Serial.println(buffer[bufferIndex]);
    Wire.write(buffer[bufferIndex]);
    ++bufferIndex; 
  }
}

void i2cSendCounter(long counter)
{
  bufferIndex = 0;
  uint8_t buffer[4] = {
    0, 0, 0, 0  };
  loadLong2Buffer(counter);
}

unsigned long i2cSetCounter()
{
  //Serial.println("Write ");

  int n = 0;
  uint8_t buffer[4] = {
    0, 0, 0, 0  };

  if(Wire.read() != 4)
    return 0;

  while(Wire.available())
  {
    buffer[n] = Wire.read();
    //Serial.println(buffer[n]);
    n++;
  } 

  unsigned int word = word(buffer[1], buffer[0]);
  unsigned long dword = word(buffer[3], buffer[2]);
  dword = dword << 16;

  return word | dword;
}

void loadLong2Buffer(long number)
{
  unsigned int value = word(number);
  buffer[0] = lowByte(value);
  buffer[1] = highByte(value);

  value = number >> 16;
  buffer[2] = lowByte(value);
  buffer[3] = highByte(value);
  //Serial.println("load in buffer");
}


// FUNCTION EPROM SAVE / READ

void saveLong(int address, unsigned long cpt, int force)
{
  //Serial.print("Save at address");
  //Serial.println(address);

  unsigned int value = word(cpt);

  //Save only X pulse
  if(force == 0 && lowByte(value) != 0) return;

  //Serial.print(" value : ");
  //Serial.println(cpt);

  EEPROM.write(address, lowByte(value));
  EEPROM.write(address+1, highByte(value));

  value = cpt >> 16;
  EEPROM.write(address+2, lowByte(value));
  EEPROM.write(address+3, highByte(value));
}

unsigned long readLong(int address)
{
  unsigned int word = word(EEPROM.read(address+1), EEPROM.read(address)); 
  unsigned long dword = word(EEPROM.read(address+3), EEPROM.read(address+2));
  dword = dword << 16;

  return dword | word;
}

