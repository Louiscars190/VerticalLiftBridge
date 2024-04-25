#include <HC_SR04.h>
#include <CheapStepper.h>
#include <BlockNot.h>

/*Lights*/
int StrobePin = 7;       // Makes a Variable for pin 7, for use as a strobe
int GreenTrafficPin = 6; // Outputs to 2 Green LEDS
int RedTrafficPin = 5;   // Outputs to 2 Red LEDS
bool LEDSTATE = false;

/*Ultrasonic Sensor Setup*/
HC_SR04_BASE *Slaves[] = { new HC_SR04<2>() }; 
HC_SR04<3> sonicMaster(4, Slaves, 1); // Master operation binding sonic object to trigger and echo pins
long Masterdistance, Slavedistance; // Number variables used to get measured distance

/*Stepper Setup*/
CheapStepper stepper (A0,A1,A2,A3);  
CheapStepper stepper2 (10,11,12,13);
bool moveClockwise = true;
unsigned long moveStartTime = 0; // this will save the time (millis()) when we started each new move
unsigned long move2StartTime = 0;

/*Switch-State Machine*/
enum Bridgestate {IDLE, MOVING};
     Bridgestate Bridgestate = 0;  // Makes an Enumeration containing IDLE and MOVING to be used in the state machine. // Starts the code with Bridgestate initially set to IDLE
bool Bridgedown = true; // Might not need

/*Non Blocking Timers*/
BlockNot strobetimer(1000);
BlockNot SerialSonicTimer(1500);
BlockNot SerialSonic2Timer(1500);
BlockNot SerialBridgeTimer(2000); // Updates Bridge Status every two seconds
BlockNot bridgetimer(4000);
BlockNot traffictimer(500);

void setup() 
{
  Serial.begin(9600); // Initializes Serial communication, Used for debugging when needed and updates throughout the process of the bridge movement
  
  stepper.setRpm(16);
  stepper2.setRpm(16);
  
  stepper.newMoveToDegree(moveClockwise, 0); // For Calbration of Initial Bridge Position
  stepper2.newMoveToDegree(moveClockwise, 0);
  
  sonicMaster.beginAsync(); // Starts measurement of both ultrasonic sensors

  pinMode(StrobePin, OUTPUT); // Setting up Strobe Pin for Output
  pinMode(RedTrafficPin, OUTPUT);   // Setting up Stop Traffic Pin for Output
  pinMode(GreenTrafficPin, OUTPUT); // Setting up Go Traffic Pin for Output
  
  Serial.println("Done Initilization, Ready for cars to come!"); // Outputs 'Done Initilization, Ready for cars to come!' to the Serial Monitor on Arduino IDE
}

void loop() 
{
  Bridgestate = IDLE;

  Masterdistance = masterdistanceread();
  Slavedistance = slavedistanceread();

  sonicMaster.startAsync(200000);
  while(!sonicMaster.isFinished()) 
  {
    stepper.run();
    stepper2.run();
    int stepsLeft = stepper.getStepsLeft();
    int steps2Left = stepper2.getStepsLeft();

    if(strobetimer.TRIGGERED) 
    {
      LEDSTATE = !LEDSTATE;
      digitalWrite(StrobePin, LEDSTATE); // Blinks Strobes every second
    }
    switch (Bridgestate)
    {
      case IDLE:
        digitalWrite(RedTrafficPin, HIGH);   // Turns on Red Traffic Lights
        digitalWrite(GreenTrafficPin, LOW);  // Turns off Green Traffic Lights
        if (SerialBridgeTimer.TRIGGERED)
        {
         Serial.print("Bridge State: ");      
         Serial.println(Bridgestate);
        }
        if (Masterdistance < 4 || Slavedistance < 4) 
        {
          Bridgestate = MOVING;
        }
        break;
      case MOVING:
        if (SerialBridgeTimer.TRIGGERED)
        {
          Serial.print("Bridge State: ");
          Serial.println(Bridgestate);
        }
          flashinglights();
        /*Moving Down*/
          moveClockwise = !moveClockwise; // reverse direction
          stepper.newMoveDegrees (moveClockwise, 180); // move 180 degrees from current position
          moveStartTime = millis(); // reset move start time
          
          stepper2.newMoveDegrees (!moveClockwise, 180); // move 180 degrees from current position
          move2StartTime = millis(); // reset move start time
        bridgetimer.start(WITH_RESET);
        if (bridgetimer.TRIGGERED)
        {
          moveClockwise = !moveClockwise; // reverse direction
          stepper.newMoveDegrees (!moveClockwise, 180); // move 180 degrees from current position
          moveStartTime = millis(); // reset move start time

          stepper2.newMoveDegrees (moveClockwise, 180); // move 180 degrees from current position
          move2StartTime = millis(); // reset move start time
        }
        if (stepsLeft || steps2Left) 
        {
        Bridgestate = MOVING;
        } 
        else 
        {
        Bridgestate = IDLE;
        }
        break;
    }
  }
}


// Reads the distance measured by the ultrasonic and can output the number to serial.
float masterdistanceread() // Outputs the distance of a master rangefinder as a function 
{
  Masterdistance = sonicMaster.getDist_cm(0);
  if(SerialSonicTimer.TRIGGERED)
  {
    Serial.print("Distance of Master: "); // Outputs Distance in centimeters to the Serial Monitor
    Serial.print(Masterdistance );
  }
  return(Masterdistance);
}

// Reads the distance measured by the ultrasonic and can output the number to serial.
float slavedistanceread() // Outputs the distance of slave rangefinder as a function 
{
  Slavedistance = sonicMaster.getDist_cm(1);
  if (SerialSonic2Timer.TRIGGERED)
  {
    Serial.print("Distance of Slave: ");
    Serial.println(Slavedistance);
  }
  return(Slavedistance);
}

void flashinglights()
{
  if (traffictimer.TRIGGERED)
  {
    digitalWrite(RedTrafficPin, !digitalRead(RedTrafficPin));
    digitalWrite(GreenTrafficPin, !digitalRead(GreenTrafficPin));
  }
  return; 
}

/*--------------END-----------*/
