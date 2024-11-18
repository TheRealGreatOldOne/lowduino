

//**************************************************//
//*                                                *//
//*  *  *  *  *  *  L O W D U I N O *  *  *  *  *  *//
//*                                                *//
//*  A microprocessor based low fuel warning and   *//
//*  fuel gauge damping system.                    *//
//*                                                *//
//*            Code Version 10.1 (Final)           *//
//*                                                *//
//*    http://www.greatoldone.co.uk/electronics/   *//
//*                                                *//
//**************************************************//

// include Timed Action Headers
#include <TimedAction.h>

// define inputs and outputs
#define G_PWM 6                  // define gauge pwm on digital pin 6
#define L_OUT_Y 4                // define yellow LED output on digital pin 2
#define L_OUT_R 2                // define red LED output on digital pin 4
#define SN_IN 7                  // define sender input on analog pin 7

const int fuelThresholdLow = 2;  // red LED turn on point (%)
const int fuelThresholdInt = 15; // yellow LED turn on point (%)

const int numReadings = 50;      // number of readings to maintain

// Array of values mapping tank sender analog levels to a percentage range

int senderMap[] = 
  // NEW MEASURED MAP
  // <- EMPTY
  { 821,819,816,813,810,807,805,804,802,801,
    799,797,795,792,790,788,786,784,782,779,
    776,772,769,765,760,755,753,751,750,749,
    746,739,736,735,731,728,726,724,721,718,
    714,709,702,696,693,690,687,683,674,660,
    657,654,651,645,639,638,635,628,623,617,
    607,597,589,587,582,574,565,555,543,531,
    519,507,498,496,484,470,457,437,411,408,
    407,406,381,348,344,325,287,276,270,263,
    238,203,181,168,171,166,160,161,155,152,149 };
  // -> FULL
  
// Array of values mapping guage PWM levels to a percentage range

int gaugeMap[] = 
  // <- EMPTY
  { 58,59,60,61,62,63,63,64,65,66,
    67,68,69,70,71,72,72,73,74,75,
    76,77,78,79,80,81,81,82,83,84,
    85,86,87,88,89,90,90,91,92,93,
    94,95,96,97,98,99,99,100,101,102,
    103,104,105,106,107,108,108,109,110,111,
    112,113,114,115,116,117,117,118,119,120,
    121,122,123,124,125,126,126,127,128,129,
    130,131,132,133,134,135,135,136,137,138,
    139,140,141,142,143,144,144,145,146,147,148 };
  // -> FULL

int sender = 0;                  // current sender reading 
int normal = 0;                  // 'normalised' sender reading (will be in range of 0 - 100)

int average = 0;                 // current average to be used for display (range 0 - 100)
int total = 0;                   // current running total (int should be fine, max will be 100 * numReadings)

int normalisedData[numReadings]; // array to hold data
int dataLocation = 0;            // where are we in the array?

// declare timed action classes for delayed processing
TimedAction taTakeSample(5000, takeSample);      // read sender signal every n milliseconds (5 seconds)
TimedAction taGaugeDisplay(1000, gaugeDisplay);  // set value to display every n milliseconds (1 Seconds)

void setup()
{
 Serial.begin(9600);
  
  // setup I/O 
  pinMode(G_PWM, OUTPUT);
  pinMode(L_OUT_Y, OUTPUT); 
  pinMode(L_OUT_R, OUTPUT);
  
  // read initial state and store
  initialValues();
  
  // set initial gauge display
  gaugeDisplay();
}

void loop()
{
  // sample the sender, and average
  // update the gauge & lamp
  // done by firing the check methods on the instantiated TA objects.
  
   taTakeSample.check();
   taGaugeDisplay.check();
}

void gaugeDisplay()
{
  // take percentage value from normalisation & averaging and turn back into pwm 
  // duty cycle. Also determine each LED status
  
  // map the duty cycle
  int dutyCycle = mapGaugeReading(average);
 
  // update gauge
  analogWrite(G_PWM, dutyCycle); 
  
  // update warning LEDs
  if (average <= fuelThresholdInt)
  {
    if (average <= fuelThresholdLow)
    {
      // Fuel low, light red LED
      digitalWrite(L_OUT_Y, LOW);
      digitalWrite(L_OUT_R, HIGH);
    }
    else
    {
      // Fuel intermediate, light yellow LED
      digitalWrite(L_OUT_Y, HIGH);
      digitalWrite(L_OUT_R, LOW);
    }
  }
  else
  {
    // both LEDs off
    digitalWrite(L_OUT_Y, LOW);
    digitalWrite(L_OUT_R, LOW);
  }
}

void takeSample()
{
  // Take Reading
  int sender = analogRead(SN_IN);
 
  // Map Reading
  int current = mapTankReading(sender);
  
  // check for end of array, if so go back to beginging before doing anything
  if (dataLocation == numReadings)
  {
    dataLocation = 0;
  }

  // deduct the oldest saved value from the total
  total = total - normalisedData[dataLocation];
 
  // overwrite oldest array location with new reading
  normalisedData[dataLocation] = current;
    
  // add new reading to the total
  total = total + current;
  
  // calaculate average
  average = total / numReadings;

  // move array pointer on one cell
  dataLocation++;
}

void initialValues()
{
    // ensure both LEDs are off (if not set, they might wander)
    digitalWrite(L_OUT_Y, LOW);
    digitalWrite(L_OUT_R, LOW);
    
    int initialReading = analogRead(SN_IN);
  
    // first reading of tank, normalise and write this to all array locations, set the total and first average 
    average = mapTankReading(initialReading);
  
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
    {
      normalisedData[thisReading] = average; 
    }
    
    // calculate the total for future use
    total = average * numReadings;
}

int mapTankReading(int reading)
{
  // take the reading, and match to values in the tank map array. Location becomes percentage value.
  int mappedReading = 0;
  boolean readingRetrieved = false;
  
  for (int readMap = 0; readMap <= 100; readMap++)
  { 
    
    if (reading >= senderMap[readMap])
    {
      // reading is greater than the value in the current array location, so return that array location ID
      // and set the read boolean to avoid the catch all 
      mappedReading = readMap;
      readingRetrieved = true;
      break;   
    }
    
    // catch all to ensure that the gauage reads full if sender value > 
    // than map, and we fall of the end of the array and no value is set.
    // this may occur if tank has been 'brimmed'
    if (!readingRetrieved)
    {
      mappedReading = readMap;
    }
  }
  
  return mappedReading;
}

int mapGaugeReading(int percent)
{
  // return the PWM duty cycle held in the array location corresponding to the percentage passed in.
  return gaugeMap[percent];
}

