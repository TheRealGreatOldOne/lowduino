
#define SN_IN 7            

int currentSample = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Enter s on serial terminal ");
  Serial.println("to read latest fuel level value");
  Serial.println("");
}


void loop()
{
  int incomingByte = 0;  
  int currentReading = 0;

  if (Serial.available() > 0) 
  {
    incomingByte = Serial.read();
  
    if (incomingByte==115) 
    {
      currentSample++;
      currentReading = analogRead(SN_IN);

      Serial.print("Sample ");
      Serial.print(currentSample);
      Serial.print(" - Value: ");
      Serial.println(currentReading);
    }

  }

}
