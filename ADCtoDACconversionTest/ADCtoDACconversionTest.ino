void setup() {
  Serial.begin(9600);
  while (!Serial);
  float vdd = 5;
  float vref = 2.048;
  uint16_t adcValue = 200; //200 ~ 0.97 v
  //conversion if we want perfect voltage conversion: 1V adc votlage turns to 1V dac voltage
  float ADCtoDACconversion1 = (vdd/1024) * (4096/vref);
  uint16_t convert1 = adcValue * ADCtoDACconversion1;
  //conversion if we want percentage voltage conversion: 100% adc voltage (vdd) turns to 100% dac voltage (2.048)
  float ADCtoDACconversion2 = 4096 / 1024;
  uint16_t convert2 = adcValue * ADCtoDACconversion2;
  Serial.println("Conversion 1");
  Serial.println(convert1);
  Serial.println("Conversion 2");
  Serial.println(convert2);
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
