#include <Arduino.h>

#include <TM1637Display.h>

const int CLK = 13; //Set the CLK pin connection to the display
const int DIO = 12; //Set the DIO pin connection to the display

int numCounter = 0;

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.

void setup()
{
  Serial.begin(115200);
display.setBrightness(0x0a); //set the diplay to maximum brightness  
                             // Set brightness (0-7):
}

void loop()
{
for(numCounter = 0; numCounter < 1000; numCounter++) //Iterate numCounter
{
  Serial.print("COUNTER :");Serial.println(numCounter);
display.showNumberDec(numCounter); //Display the numCounter value;
delay(100);
}
}