#include <SPI.h>

// Pin definitions
float vddADC = 3.3;
float vrefDAC = 2.048;

const float ADCtoDACconversion1 = (vddADC/1024) * (4096/vrefDAC); //pure voltage conversion: 1v ADC to 1v DAC
const float ADCtoDACconversion2 = 4096 / 1024; // percentage conversion: 50% of vdd ADC voltage to 50% of vref DAC voltage

//pins
const int ADC_CS = 10;  // Chip select for MCP3002 ADC
const int DAC_CS = 9;   // Chip select for MCP4821 DAC

//timing
unsigned long timeNow = micros();
unsigned long timeLast = micros();
unsigned long samplingPeriod = 63;

//speed tests
uint16_t sig = 2000;
/*
const int iterations = 10000;
unsigned long adc[iterations];
unsigned long convert[iterations];
unsigned long dac[iterations];
*/

void setup() {
  // Initialize SPI
  SPI.begin();
  
  // Set the SPI settings (Clock speed, bit order, data mode)
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  // Set chip select pins as outputs
  pinMode(ADC_CS, OUTPUT);
  pinMode(DAC_CS, OUTPUT);
  
  // Start with both CS high
  digitalWrite(ADC_CS, HIGH);
  digitalWrite(DAC_CS, HIGH);

  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  
  

  /*
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
  */
  

}

void loop() {
  
  timeNow = micros();
  if ((timeNow-timeLast)>= samplingPeriod){
    //square wave
    /*
    if (sig == 2000){
      writeDAC(2000);
      sig = 1000;
    }
    else{
      writeDAC(1000);
      sig = 2000;
    }
    */
    //uint16_t adcValue = readADC();       // 10-bit value (0–1023)
    //float voltage = (adcValue / 1023.0) * 3.3;  // Convert ADC reading to volts (assuming 3.3 V Vref)
    //uint16_t dacValue = (voltage / 4.096) * 4095;  // Convert to DAC value assuming gain = 1
    
    
    uint16_t adcValue = readADC();
    Serial.println(adcValue);
    delayMicroseconds(10);
    writeDAC(adcValue);
    //Serial.println(adcValue);
    //uint16_t dacValue = adcValue*ADCtoDACconversion1;
    //writeDAC(dacValue);
    
    timeLast = timeNow;
  }

  


  
  // Read data from MCP3002 (ADC)
  //uint16_t adcValue = readADC();
  //Serial.println(adcValue);
  //writeDAC(2000);
  /*
  // Print the ADC value for monitoring
  Serial.print("ADC Value: ");
  Serial.println(adcValue);

  uint16_t dacValue = adcValue*ADCtoDACconversion1;

  Serial.print("DAC Value: ");
  Serial.println(dacValue);

  // Pass the ADC value to MCP4821 (DAC)
  writeDAC(dacValue);
  */

  //delay(1000); // Delay for stability
}


  // Reads from MCP3002 channel 0 (single-ended mode)
uint16_t readADC() {
  digitalWrite(ADC_CS, LOW);

  // MCP3002 control bits:
  // Start bit = 1
  // SGL/DIFF = 1 (single-ended)
  // ODD/SIGN = 0 (channel 0)
  byte command = 0b11010000; // Start + single-ended, CH0

  byte highByte = SPI.transfer(command);
  byte lowByte = SPI.transfer(0x00);

  digitalWrite(ADC_CS, HIGH);

  // Combine to 10-bit result (discard null bit)
  uint16_t result = ((highByte & 0x03) << 8) | lowByte;
  return result;
}

// Writes a 12-bit value to MCP4821 DAC
void writeDAC(uint16_t value) {
  value &= 0x0FFF;  // Ensure 12-bit range

  // Control bits:
  // 0b0011 = Write, Unbuffered, Gain 1x, Active
  uint16_t command = (0b0011 << 12) | value;

  byte highByte = (command >> 8) & 0xFF;
  byte lowByte = command & 0xFF;

  digitalWrite(DAC_CS, LOW);
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  digitalWrite(DAC_CS, HIGH);
}


/*
// Function to read the ADC (MCP3002)
uint16_t readADC() {
  digitalWrite(ADC_CS, LOW);  // Activate the ADC

  // MCP3002 is a 10-bit ADC, so read 2 bytes
  byte highByte = SPI.transfer(0x60);  // Start conversion (send dummy byte)
  byte lowByte = SPI.transfer(0);   // Receive the result
  
  digitalWrite(ADC_CS, HIGH);  // Deactivate the ADC

  // Combine the two bytes (10-bit result)
  uint16_t result = ((highByte & 0xFF) << 8) | lowByte;  // Mask and combine
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
*/
