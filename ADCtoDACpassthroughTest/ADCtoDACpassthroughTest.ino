#include <SPI.h>

// Pin definitions
float vddADC = 3.3;
float vrefDAC = 2.048;

const float ADCtoDACconversion1 = (vddADC/1024) * (4096/vrefDAC); //pure voltage conversion: 1v ADC to 1v DAC
const float ADCtoDACconversion2 = 4096 / 1024; // percentage conversion: 50% of vdd ADC voltage to 50% of vref DAC voltage

//speed tests
const int iterations = 10000;
unsigned long timeNow;
unsigned long timeLast;
unsigned long adc[iterations];
unsigned long convert[iterations];
unsigned long dac[iterations];


void setup() {
  // Initialize SPI
  SPI.begin();
  
  // Set the SPI settings (Clock speed, bit order, data mode)
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));

  // Set chip select pins as outputs
  pinMode(ADC_CS, OUTPUT);
  pinMode(DAC_CS, OUTPUT);
  
  // Start with both CS high
  digitalWrite(ADC_CS, HIGH);
  digitalWrite(DAC_CS, HIGH);

  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial);


  
  //timing tests:
  //run 10000 samples through the system and record times
  timeLast = micros();
  for(int i = 0; i < iterations; i++){
    //adc
    uint16_t adcValue = readADC();
    timeNow = micros();
    adc[i] = timeNow - timeLast;
    timeLast = timeNow;
    //conversion
    uint16_t dacValue = adcValue*ADCtoDACconversion1;
    timeNow = micros();
    convert[i] = timeNow - timeLast;
    timeLast = timeNow;
    //dac
    writeDAC(dacValue);
    timeNow = micros();
    dac[i] = timeNow - timeLast;
    timeLast = timeNow;
  }
  //average times
  unsigned long adcSum = 0;
  unsigned long convertSum = 0;
  unsigned long dacSum = 0;
  for(int i = 0; i < iterations; i++){
    adcSum += adc[i];
    convertSum += convert[i];
    dacSum += dac[i];
  }
  float adcAv = float(adcSum) / float(iterations);
  float convertAv = float(convertSum) / float(iterations);
  float dacAv = float(dacSum) / float(iterations);
  //print results
  Serial.print("Average time for ADC read:");
  Serial.println(adcAv);
  Serial.print("Total time for 10000 ADC reads:");
  Serial.println(adcSum);

  Serial.print("Average time for ADC to DAC conversion:");
  Serial.println(convertAv);
  Serial.print("Total time for 10000 ADC to DAC conversions:");
  Serial.println(convertSum);

  Serial.print("Average time for DAC write:");
  Serial.println(dacAv);
  Serial.print("Total time for 10000 DAC writes:");
  Serial.println(dacSum);
  
  

}

void loop() {
  /*
  // Read data from MCP3002 (ADC)
  uint16_t adcValue = readADC();

  // Print the ADC value for monitoring
  Serial.print("ADC Value: ");
  Serial.println(adcValue);

  uint16_t dacValue = adcValue*ADCtoDACconversion1;

  Serial.print("DAC Value: ");
  Serial.println(dacValue);

  // Pass the ADC value to MCP4821 (DAC)
  writeDAC(dacValue);
  */

  delay(300); // Delay for stability
}

// Function to read the ADC (MCP3002)
uint16_t readADC() {
  digitalWrite(ADC_CS, LOW);  // Activate the ADC

  // MCP3002 is a 10-bit ADC, so read 2 bytes
  byte highByte = SPI.transfer(0x60);  // Start conversion (send dummy byte)
  byte lowByte = SPI.transfer(0);   // Receive the result
  
  digitalWrite(ADC_CS, HIGH);  // Deactivate the ADC

  // Combine the two bytes (10-bit result)
  uint16_t result = ((highByte & 0x03) << 8) | lowByte;  // Mask and combine
  return result;
}

// Function to send data to DAC (MCP4821)
void writeDAC(uint16_t value) {
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
