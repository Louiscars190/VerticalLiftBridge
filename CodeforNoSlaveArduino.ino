#include <CheapStepper.h>
#include <BlockNot.h>

/*Lights*/
int StrobePin = 7;       // Makes a Variable for pin 7, for use as a strobe
int GreenTrafficPin = 6; // Outputs to 2 Green LEDS
int RedTrafficPin = 5;   // Outputs to 2 Red LEDS
bool LEDSTATE = false;

/*Photoresistor*/
int sensor1value = 0;
int sensor2value = 0;
int baseline = 0;
int baseline1 = 0;

/*Stepper Setup*/
CheapStepper stepper (4,5,6,7);  
CheapStepper stepper2 (8,9,10,11);
bool moveClockwise = true;

/*Switch-State Machine*/
enum Bridgestate {UP, MOVING};
     Bridgestate Bridgestate = UP;  // Makes an Enumeration containing IDLE and MOVING to be used in the state machine. // Starts the code with Bridgestate initially set to IDLE
bool Bridgedown = true; // Might not need

/*Non Blocking Timers*/
BlockNot strobetimer(1000);
BlockNot SerialSensorTimer(1500);
BlockNot SerialBridgeTimer(2000); // Updates Bridge Status every two seconds
BlockNot bridgetimer(4000);
BlockNot traffictimer(500);
BlockNot sensortimer(5000);

void setup() 
{
  Serial.begin(9600); // Initializes Serial communication, Used for debugging when needed and updates throughout the process of the bridge movement
  
  stepper.setRpm(16);
  stepper2.setRpm(16);
  
  pinMode(A0, INPUT); // Setting up Photoresistor Sensor
  pinMode(A1, INPUT); // Setting up Second Sensor (Slave Side)
  baseline = analogRead(A0);
  baseline1 = analogRead(A1);
  Serial.print("Baseline 1 Calibration: ");
  Serial.println(baseline);
  Serial.print("Baseline 2 Calibration: ");
  Serial.println(baseline1);

  pinMode(StrobePin, OUTPUT); // Setting up Strobe Pin for Output
  pinMode(RedTrafficPin, OUTPUT);   // Setting up Stop Traffic Pin for Output
  pinMode(GreenTrafficPin, OUTPUT); // Setting up Go Traffic Pin for Output
  
  Serial.println("Done Initilization, Ready for cars to come!"); // Outputs 'Done Initilization, Ready for cars to come!' to the Serial Monitor on Arduino IDE
}

void loop() 
{
  stepper.run();
  stepper2.run();
  
  int sensor1value = analogRead(A0);
  int sensor2value = analogRead(A1);


  unsigned int sensordelta = sensor1value - baseline;
  unsigned int sensor1delta = sensor2value - baseline1;
  switch (Bridgestate)
  {
    case UP:
      Bridgestate = UP;
      digitalWrite(RedTrafficPin, HIGH);   // Turns on Red Traffic Lights
      digitalWrite(GreenTrafficPin, LOW);  // Turns off Green Traffic Lights
      if (SerialBridgeTimer.TRIGGERED)
      {
        Serial.print("Bridge State: "); 
        Serial.println(Bridgestate);
        Serial.print("Sensor Delta 1: ");
        Serial.println(sensordelta);
        Serial.print("Sensor Delta 2: ");
        Serial.println(sensor1delta);
      }
      if (sensordelta > 30 || sensor1delta > 30) 
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
      /*Moving Down*/
        moveClockwise = !moveClockwise; // reverse direction
        stepper.moveDegrees (moveClockwise, 360); // move 180 degrees from current position
        stepper2.newMoveDegrees (!moveClockwise, 360); // move 180 degrees from current position

        digitalWrite(GreenTrafficPin, HIGH);
        digitalWrite(RedTrafficPin, LOW);

        delay(1000);

        moveClockwise = !moveClockwise; // reverse direction
        stepper.newMoveDegrees (!moveClockwise, 360); // move 180 degrees from current position

        stepper2.newMoveDegrees (moveClockwise, 360); // move 180 degrees from current position

        delay (1000);
    
        Bridgestate = UP;
    break;
  }
}

/*--------------END-----------*/
