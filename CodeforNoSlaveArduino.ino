#include <Stepper.h>
#include <Servo.h>
#include <Ultrasonic.h> // Make sure to install ultrasonic library if you haven't already in your Arduino IDE

const int stepsPerRev = 2038; // Steps per Rotation

Stepper MasterStepper = Stepper(stepsPerRev, 9,10,11,12); // Stepper is the constructor of the Stepper class. myStepper is the object we initialize. ULN2003: IN1, IN2, IN3, IN4 digital pin 9,10,11,12
Stepper SlaveStepper  = Stepper(stepsPerRev, A0,A1,A2,A3);// Stepper libray sets pins as digital outputs so we do not need to do it in void setup()
Servo Masterservo; // Creating master-side servo object 
Servo Slaveservo;  // Creating slave-side servo object

int MasterservoPin = 4; // Makes a Variable for pin 4, for use as a servo
int SlaveservoPin = 8;  // Makes a Variable for Pin 8, for use as a servo

int StrobePin = 7;       // Makes a Variable for pin 7, for use as a strobe
int GreenTrafficPin = 6; // Outputs to 2 Green LEDS
int RedTrafficPin = 5;   // Outputs to 2 Red LEDS

Ultrasonic MasterUltrasonic(0,2); // Makes the Master-side Ultrasonic Object with Trigger pin 0 and Echo Pin 2
Ultrasonic SlaveUltrasonic(1,3);  // Makes the Slave-side Ultrasonic Object with Trigger pin 1 and Echo Pin 3

float Masterdistance, Slavedistance;

enum Bridgestate {IDLE, MOVING};  // Makes an Enumeration containing IDLE and MOVING to be used in the state machine.
     Bridgestate = IDLE; // Starts the code with Bridgestate initially set to IDLE
bool Bridgedown = true;

unsigned long currentmillis = millis(); // Starts the clock for current millis, can use it to update various differnt 'previousmillis' variables
unsigned long previousmillis = 0;
const long strobeint = 500;        // Change the interval if you want to change how fast the strobes blink
bool LEDSTATE = false;            // Set LEDSTATE to false initially

const long bridgeint = 10000;     // How long the bridge will stay down for before going back up
unsigned long bridgeprevmill = 0; // Initilizating previous millisecond variables for bridge so that it stays down for 10 seconds

const long trafficint = 500;        // Initializing constant interval for green traffic light
unsigned long trafficprevmill = 0;  // Initializing previous millisecond variables for green traffic interval

//DONE
void setup() 
{
  Serial.begin(9600); // Initializes Serial communication, Used for debugging when needed and updates throughout the process of the bridge movement
  
  MasterStepper.setSpeed(10);
  MasterStepper.step(1019); // Need to find out how much rotation we need to raise the entire thing, possiblt more than a full turn which means we will need a gear reduction. As a placeholder we will put half of the mapped rotation.
  
  SlaveStepper.setSpeed(10);
  SlaveStepper.step(1019); // If needed, put a negative will make it go the other way.
  
  Masterservo.attach(MasterservoPin); // Attach Masterservo to pin 'MasterservoPin'
  Slaveservo.attach(SlaveservoPin);   // Atach Slaveservo to pin 'SlaveservoPin'
  
  Masterservo.write(90); // Could be 180, depending on how we attach the servo, change later
  Slaveservo.write(90);  // || // FOR CLOSED BARRIER (DOWN)
  
  pinMode(StrobePin, OUTPUT); // Setting up Strobe Pin for Output
  pinMode(RedTrafficPin, OUTPUT);   // Setting up Stop Traffic Pin for Output
  pinMode(GreenTrafficPin, OUTPUT); // Setting up Go Traffic Pin for Output
  
  Serial.println("Done Initilization, Ready for cars to come!"); // Outputs 'Done Initilization, Ready for cars to come!' to the Serial Monitor on Arduino IDE
}

void loop() // This is our main loop
{ 
  Masterdistance = masterdistanceread(); // Update variable with current distance read from sensor
  Slavedistance = slavedistanceread();   // ||

  if (currentmillis - previousmillis >= strobeint) // Not sure <= or >= check later, change for the blinking traffic lights too
  {
    previousmillis = currentmillis;
    LEDSTATE = !LEDSTATE;
    digitalWrite(StrobePin, LEDSTATE);
  }

  switch (Bridgestate) // IDLE and MOVING, two states of the Bridge system
  {
    case IDLE: // Trafffic Light is Red, Listening for Car. If car there, change state to moving.
      digitalWrite(RedTrafficPin, HIGH);   // Turns on Red Traffic Lights
      digitalWrite(GreenTrafficPin, LOW);  // Turns off Green Traffic Lights
      Serial.print("Bridge State: ");      // Outputs Bridge State to Serial Monitor for Debugging purposes
      Serial.println(Bridgestate);
      if (Masterdistance <= 2)
      {
        Bridgestate = MOVING;
      }
      break;
    case MOVING:
      Serial.print("Bridge State: "); // Tells us the state of the bridge on the serial monitor (Debugging purposes)
      Serial.println(Bridgestate);    // Prints the current case of 'Bridgestate'
      flashinglights(); // Flashes the RED and GREEN LEDS from variables 'GreenTrafficPin' and 'RedTrafficPin' Only when moving Green and Red pins
      movebridge();     // Moves the down for the cars to pass, waits (milliseconds, or delay), then moves it back up // open Barrier nested in move bridge?
      Bridgestate = IDLE;
      break;
  }
}

//DONE
float masterdistanceread() // Outputs the distance of a master rangefinder as a function 
{
  Masterdistance = MasterUltrasonic.read();
  Serial.print("Distance of Master: "); // Outputs Distance in centimeters to the Serial Monitor
  Serial.println(Masterdistance);
  return(Masterdistance);
}

//DONE
float slavedistanceread() // Outputs the distance of slave rangefinder as a function
{
  Slavedistance = SlaveUltrasonic.read();
  Serial.print("Distance of Slave: ");
  Serial.println(Slavedistance);
  return(Slavedistance);
}

//DONE
void closebarrier() 
{
  Masterservo.write(90); // Could be 180, depending on how we attach the servo, change later
  Slaveservo.write(90);  // ||
  Serial.print("Barrier = Closed?: ");
  Serial.println("True");
  return;
}

//DONE
void openbarrier()
{
  Masterservo.write(0); // Could be 180, depending on how we attach the servo, change later
  Slaveservo.write(0);  // ||
  digitalWrite(GreenTrafficPin, HIGH);
  digitalWrite(RedTrafficPin, LOW);
  Serial.print("Barrier = Closed?: ");
  Serial.println("False");
  return; // Opens the barrier so the cars can pass (same delay as when the bridge is in its lowered position) and sets Traffic lights to green
}

//DONE
void flashinglights() // Skeptical about this part of the code //DONE
{
  
  unsigned long trafficElapsed = currentmillis - trafficprevmill;
  if (trafficElapsed >= trafficint && Bridgedown == false) // Only works when the bridge is not fully down, because when it is the light will turn green (inside the open barier function)
  {
    trafficprevmill = currentmillis;

    digitalWrite(RedTrafficPin, !digitalRead(RedTrafficPin));
    digitalWrite(GreenTrafficPin, !digitalRead(GreenTrafficPin));
  }
  return; // Flashes lights while the bridge is moving
}




//NOT DONE
void movebridge() // Most important function in this whole thing 
{
  unsigned long bridgeElapsed = currentmillis - bridgeprevmill;

  //bridge move
            // Stepper motor movement here // Apparently this is a blocking function...Pain...dont worry the others are non blocking it should work
  
  bridgeprevmill = currentmillis; // Set Previous Millisecond to Current time to reset, once bridge is down
  
  //something that starts the while thing
  while (bridgeElapsed <= bridgeint) // Basically does nothing for 10 seconds 
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

/*--------------END-----------*/

