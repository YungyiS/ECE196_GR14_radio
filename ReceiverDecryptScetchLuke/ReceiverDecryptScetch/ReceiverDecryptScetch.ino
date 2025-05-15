#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(7, 8, 10000000); // CE, CSN

const byte address[6] = "00001";

const int iterations = 100;

unsigned long timeNow = micros();
unsigned long timeLast = micros();
unsigned long times[iterations];
unsigned long timesTotal = 0;
float timesAverage;

unsigned long timeNow2 = micros();
unsigned long timeLast2 = micros();
unsigned long times2[iterations];
unsigned long timesTotal2 = 0;
float timesAverage2;

int j = 0;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setDataRate(RF24_1MBPS); 
  radio.setChannel(50);
  //SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  radio.setRetries(1, 15);
}

void loop() {
  if ((radio.available()) && (j < iterations)) {
    timeLast2 = micros();

    uint16_t text[16];
    radio.read(&text, 32);
    Serial.println("Transmission:");
    for(int i = 0; i < 16; i++){
      Serial.println(text[i]);
    }

    timeNow2 = micros();
    times2[j] = timeNow2 - timeLast2;

    //Serial.println(text);
    timeNow = micros();
    times[j] = timeNow - timeLast;
    timeLast = timeNow;
    j++;
}
if (j == iterations){
  j = 0;
  timesTotal = 0;
  timesTotal2 = 0;
  for(int i = 0; i < iterations; i++){
    timesTotal += times[i];
    timesTotal2 += times2[i];
  }
  timesAverage = float(timesTotal) / float(iterations);
  timesAverage2 = float(timesTotal2) / float(iterations);
  Serial.print("Average transmit time: ");
  Serial.println(timesAverage);
  Serial.print("Average read time: ");
  Serial.println(timesAverage2);
}

}
//hardcoded symetric key encryption function
void T1encrypt(uint16_t* ADCOutput, size_t length, uint16_t key) {
  for (size_t i = 0; i < length; i++) {
    ADCOutput[i] ^= (key >> (i % 16)) | (key << (16 - (i % 16)));
  }
}

void T1Decrypt(uint16_t* TransmittedData, size_t length, uint16_t key) {
  for (size_t i = 0; i < length; i++) {
    TransmittedData[i] ^= (key >> (i % 16)) | (key << (16 - (i % 16)));
  }
}
