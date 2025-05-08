#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(7, 8, 10000000); // CE, CSN

const byte address[6] = "00001";

void setup() {
  // put your setup code here, to run once:
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(50);
  radio.setDataRate(RF24_1MBPS);
  radio.setRetries(0, 1);  
  radio.stopListening();
  
}

uint16_t counter[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

void loop() {
  radio.write(&counter, 32);  // Send the current counter as a single byte
  delay(500);

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
