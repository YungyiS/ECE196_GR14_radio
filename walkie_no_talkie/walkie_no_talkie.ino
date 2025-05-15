#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


//pins
const int ADC_CS = 10;  // Chip select for MCP3002 ADC
const int DAC_CS = 9;   // Chip select for MCP4821 DAC
//General & timing
unsigned long ticktick = micros();
const unsigned long samplingPeriod = 63;
uint8_t bufferLocation = 0;       //variable to keep track of buffer location
const uint8_t transmitBytes = 1;  // how many bytes we're transmitting
const uint8_t transferSize = 32 / transmitBytes;
uint8_t dataBuffer[transferSize]; // input/output buffer
uint8_t dataBufferOld[transferSize];
//RF
RF24 radio(7, 8, 10000000);  // CE, CSN
const byte address[6] = "00001";
bool recieved = false;      //variable to make sure we don't output duplicates to the DAC
bool transmitting = false;  //variable to tell when we are transmitting so we dont play audio at the same time
//ADC

//DAC
uint8_t bufferOut[32];


void setup() {
  //open serial moniter
  Serial.begin(9600);

  //RF setup
  radio.begin();
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(50);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  radio.setRetries(1, 15);
}


void loop() {
  //check if we are transmitting or receiving
  if (transmitting) {
    radio.stopListening();
    //stay in transmitting mode until button is no longer being pressed
    while (transmitting) {
        //read buffer
        *dataBuffer = readADCtoBuffer(dataBuffer);
        //transmit buffer
        radio.write(&dataBuffer, transferSize);
      }
    radio.startListening();
  } else {
    //check for transmission
    if ((radio.available()) && (!recieved)) {
      radio.read(&bufferOut, 32);
      recieved = true;
    }
    //write buffer
  
    for(int i = 0; i < transferSize; i++){
      if( (micros() - ticktick) > samplingPeriod){
        writeDAC(dataBuffer[i]);//write DAC
        ticktick = micros();
        i++;
      }
      i--;
    }
    
    
  }
}


//CUSTOM FUNCTIONS:
// Function to read the ADC (MCP3002)data by opening, reading, and closing SPI connection
uint8_t readADC() {
  digitalWrite(ADC_CS, LOW);  // Activate the ADC

  // MCP3002 is a 10-bit ADC, so read 2 bytes
  byte highByte = SPI.transfer(0x60);  // Start conversion (send dummy byte)
  byte lowByte = SPI.transfer(0);      // Receive the result

  digitalWrite(ADC_CS, HIGH);  // Deactivate the ADC

  // Combine the two bytes (10-bit result)
  uint8_t result = uint8_t((((highByte & 0x03) << 8) | lowByte) / 257);  // Mask,combine, and convert to 8 bit
  return result;
}


// Function to send data to DAC (MCP4821)
void writeDAC(uint8_t input) {
  //convert by to 16 bits
  uint16_t value = uint16_t(input * 257);
  // Limit value to 12 bits just in case
  value = value & 0x0FFF; 

  // Now set up control bits:
  uint16_t command = 0;
  command |= (0b0011 << 12); // 
  // 0b0011 = 0b0 (write to DAC) 0b0 (unbuffered) 0b1 (gain = 1x) 0b1 (active mode)

  command |= value;  // Merge the 12-bit DAC value into the lower bits

  byte highByte = (command >> 8) & 0xFF;  // Top 8 bits
  byte lowByte = command & 0xFF;          // Bottom 8 bits

  digitalWrite(DAC_CS, LOW);
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  digitalWrite(DAC_CS, HIGH);
}


// Function to fill the buffer using data from the ADC --- need to fix memory address shanigans
int readADCtoBuffer(uint8_t* dataBuffer) {
  //read data until buffer is full
  uint8_t i = 0;
  dataBuffer = readADC();
  unsigned long ticktick = micros();
  i++;
  if (i == transfer) {
    return dataBuffer[i];
  }
  if ((micros() - ticktick) > samplingPeriod) {
    dataBuffer[i] = readADC();
    i++;
  }
}

