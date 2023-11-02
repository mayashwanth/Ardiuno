#define CLK 2
#define DT 3
#define SW 4
unsigned char readBits = 0b111;     // 8 bits
unsigned char rotationBits = 0b011; // 8 bits
int counter = 0;
void setup()
{
 // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  // Attach interrupts to the CLK and DT pins
  attachInterrupt(digitalPinToInterrupt(CLK), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT), updateEncoder, CHANGE);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
}
void updateEncoder()
{
  readBits = digitalRead(CLK) << 1 | digitalRead(DT);
  if ((rotationBits & 0b11) != (readBits & 0b11))
  {
    rotationBits = rotationBits << 2 | readBits & 0b11;
    if (rotationBits == 0b01001011 || rotationBits == 0b10000111)
    {
        if (rotationBits == 0b01001011)   // 11 | 01 | 00 | 10  for CCW  
        {
          if (counter > 0)
          counter--;
        }
        else if (counter <= 254)
        {
          counter++;
        }
        int a = 0;
        a = (counter >> 8) & 0xFF;
        int b = 0;
        b = counter & 0xFF;
        byte data[]={a,b};
        
        Serial.write(data,sizeof(data));
        delay(1000);
    }
  }
}
void loop() {
// put your main code here, to run repeatedly:
}