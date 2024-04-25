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
enum Bridgestate {IDLE, MOVING};  // Makes an Enumeration containing IDLE and MOVING to be used in the state machine.
     Bridgestate Bridgestate = IDLE; // Starts the code with Bridgestate initially set to IDLE
bool Bridgedown = true; // Might not need

/*Non Blocking Timers*/
BlockNot strobetimer(1000);
BlockNot SerialSonicTimer(750);
BlockNot SerialBridgeTimer(2000); // Updates Bridge Status every two seconds
BlockNot bridgetimer(10000);
BlockNot traffictimer(500);

/*
unsigned long currentmillis = millis(); // Starts the clock for current millis, can use it to update various differnt 'previousmillis' variables
unsigned long previousmillis = 0;
const long strobeint = 1000;        // Change the interval if you want to change how fast the strobes blink

const long bridgeint = 10000;     // How long the bridge will stay down for before going back up
unsigned long bridgeprevmill = 0; // Initilizating previous millisecond variables for bridge so that it stays down for 10 seconds

const long serialupdateint = 950;

const long trafficint = 500;        // Initializing constant interval for traffic light
unsigned long trafficprevmill = 0;  // Initializing previous millisecond variables for green traffic interval
*/

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
  Masterdistance = masterdistanceread(); // Update variable with current distance read from sensor
  Slavedistance = slavedistanceread();   // ||

   if(strobetimer.TRIGGERED) 
  {
    LEDSTATE = !LEDSTATE;
    digitalWrite(StrobePin, LEDSTATE); // Blinks Strobes every second
  }
  switch (Bridgestate) // IDLE and MOVING, two states of the Bridge system
  {
    case IDLE: // Trafffic Light is Red, Listening for Car. If car there, change state to moving.
      digitalWrite(RedTrafficPin, HIGH);   // Turns on Red Traffic Lights
      digitalWrite(GreenTrafficPin, LOW);  // Turns off Green Traffic Lights
      if (SerialBridgeTimer.TRIGGERED)
      {
        Serial.print("Bridge State: ");      // Outputs Bridge State to Serial Monitor for Debugging purposes
        Serial.println(Bridgestate);
      }
      if (Masterdistance <= 4 || Slavedistance <= 4) // USED MORE THAN TO TEST STATE #0
      {
        Bridgestate = MOVING;
      }
      break;
    case MOVING:
      if(SerialBridgeTimer.TRIGGERED)
      {
        Serial.print("Bridge State: "); // Tells us the state of the bridge on the serial monitor (Debugging purposes)
        Serial.println("Moving");    // Prints the current case of 'Bridgestate'
      }
      flashinglights(); // Flashes the RED and GREEN LEDS from variables 'GreenTrafficPin' and 'RedTrafficPin' Only when moving Green and Red pins
      
      //{
      //  Bridgestate = IDLE;
      //}
      //Bridgestate = IDLE;
      break;
  }
}

// Reads the distance measured by the ultrasonic and can output the number to serial.
float masterdistanceread() // Outputs the distance of a master rangefinder as a function 
{
  Masterdistance = sonicMaster.getDist_cm(1);
  //Serial.print("Distance of Master: "); // Outputs Distance in centimeters to the Serial Monitor
  //Serial.println(Masterdistance);
  return(Masterdistance);
}

// Reads the distance measured by the ultrasonic and can output the number to serial.
float slavedistanceread() // Outputs the distance of slave rangefinder as a function 
{
  Slavedistance = sonicMaster.getDist_cm(2);
  //Serial.print("Distance of Slave: ");
  //Serial.println(Slavedistance);
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

/*void movebridge()
{
  
  
  if (bridgetimer.TRIGGER) // Basically does nothing for 10 seconds 
  {
    Bridgedown = true;
    Serial.println("The Bridge is currently down, please proceed!");
    openbarrier();
  }
  Bridgedown = false; // Right after the wait is over
  closebarrier();   // Closes the barriers on the road which is default
  // Move stepper to up
  return; // Moves Bridge
}
*/
/*--------------END-----------*/


