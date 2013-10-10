#include <Wire.h>
#include <AccelStepper.h> // http://www.airspayce.com/mikem/arduino/AccelStepper/

//address table
#define STP_RUN_STATE 0x91
#define STP_SPEED_RL1 0x92
#define STP_SPEED_RL2 0x93
#define STP_SPEED_RH1 0x94
#define STP_SPEED_RH2 0x95
#define STP_DIS_TOGO_RL1 0x96
#define STP_DIS_TOGO_RL2 0x97
#define STP_DIS_TOGO_RH1 0x98
#define STP_DIS_TOGO_RH2 0x99
#define STP_TARGET_POS_RL1 0x9A
#define STP_TARGET_POS_RL2 0x9B
#define STP_TARGET_POS_RH1 0x9C
#define STP_TARGET_POS_RH2 0x9D
#define STP_CURRENT_POS_RL1 0x9E
#define STP_CURRENT_POS_RL2 0x9F
#define STP_CURRENT_POS_RH1 0xA0
#define STP_CURRENT_POS_RH2 0xA1

#define STP_ACC_L1 0x01
#define STP_ACC_L2 0x02
#define STP_ACC_H1 0x03
#define STP_ACC_H2 0x04
#define STP_MAX_SPEED_L1 0x05
#define STP_MAX_SPEED_L2 0x06
#define STP_MAX_SPEED_H1 0x07
#define STP_MAX_SPEED_H2 0x08
#define STP_SPEED_L1 0x09
#define STP_SPEED_L2 0x0A
#define STP_SPEED_H1 0x0B
#define STP_SPEED_H2 0x0C
#define STP_MOVE_TO_L1 0x0D
#define STP_MOVE_TO_L2 0x0E
#define STP_MOVE_TO_H1 0x0F
#define STP_MOVE_TO_H2 0x10
#define STP_MOVE_L1 0x11
#define STP_MOVE_L2 0x12
#define STP_MOVE_H1 0x13
#define STP_MOVE_H2 0x14
#define STP_CURRENT_POS_L1 0x15
#define STP_CURRENT_POS_L2 0x16
#define STP_CURRENT_POS_H1 0x17
#define STP_CURRENT_POS_H2 0x18

#define STP_RUN_CTRL 0x1C
#define STP_EN_CTRL 0x1D
#define STP_SLEEP_CTRL 0x1F
#define STP_MS_CTRL 0x20

//data table
#define STP_RUN 0x01
#define STP_STOP 0x02
#define STP_WAIT 0x03
#define STP_RESET_CTRL 0x04
#define STP_RUN_SPEED 0x05
#define STP_TRUE 0x01
#define STP_FALSE 0x00
#define STP_ENABLE 0x01
#define STP_DISABLE 0x02

#define STP_FULL 0x01
#define STP_HALF 0x02
#define STP_QUARTER 0x04
#define STP_EIGHTH 0x08
#define STP_SIXTEENTH 0x16

//pin define
#define ENABLE 7
#define SLEEP 5
#define RESET 6
#define DIR 8
#define STEP 9
#define MS1 12
#define MS2 11
#define MS3 10
#define S1 2
#define S2 3
//#define ADC7 A7
AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR); // 9-PUL, 8-DIR

int phyAddress = 4;

//limit 16777215
long stepperAcceleration = 0;
long stepperMaxSpeed = 0;
long stepperSpeed = 0;
long stepperMoveTo = 0;
long stepperMove = 0;
long stepperCuttentPos = 0;

long stepperSpeedRead = 0;
long stepperDistanceToGoRead = 0;
long stepperTargetPositionRead = 0;
long stepperCurrentPositionRead = 0;

byte stepperRunState = 0;
byte stepperRunCtrl = STP_WAIT;

void setup()
{
  pinMode(ENABLE, OUTPUT);
  pinMode(SLEEP, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
//  pinMode(S1, OUTPUT);
//  pinMode(S2, OUTPUT);
//  pinMode(A7, INPUT);

  digitalWrite(ENABLE, HIGH);  //motor disable
  digitalWrite(SLEEP, HIGH);  //
  digitalWrite(RESET, HIGH);  //
  digitalWrite(MS1, HIGH);  //
  digitalWrite(MS2, HIGH);  //
  digitalWrite(MS3, HIGH);  //
  
  Wire.begin(phyAddress);       // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
//  Serial.begin(9600);          // start serial for output
}

void loop()
{
  switch(stepperRunCtrl)
  {
    case STP_RUN: if(stepper.run()) stepperRunState = STP_TRUE;
                  else stepperRunState = STP_FALSE;
                  break;
    case STP_STOP: stepper.stop();
                   stepperRunCtrl = STP_WAIT;
                   break;
    case STP_RESET_CTRL: stepper.stop();
                         stepper.setCurrentPosition(0); 
                         stepperRunCtrl = STP_WAIT;
                         break;
    case STP_RUN_SPEED: stepper.runSpeed(); break;
    default: break;
  }
}

byte readCount = 0;
byte rxBuf[2] = {0};
byte i = 0;
// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  rxBuf[i++] = Wire.read();
  if(rxBuf[0] > 0x90) i=0;
  else
  {
    if(readCount++)
    {
      switch(rxBuf[0])
      {
        case STP_EN_CTRL: if(rxBuf[1] == STP_ENABLE) digitalWrite(ENABLE, LOW);  //motor enable
                          if(rxBuf[1] == STP_DISABLE) digitalWrite(ENABLE, HIGH);  //motor disable
                          break; 
                          
        case STP_MS_CTRL: if(rxBuf[1] == STP_FULL){digitalWrite(MS1, LOW); digitalWrite(MS2, LOW); digitalWrite(MS3, LOW);}       //Full Step
                          else if(rxBuf[1] == STP_HALF){digitalWrite(MS1, HIGH); digitalWrite(MS2, LOW); digitalWrite(MS3, LOW);}  //Half Step
                          else if(rxBuf[1] == STP_QUARTER){digitalWrite(MS1, LOW); digitalWrite(MS2, HIGH); digitalWrite(MS3, LOW);}  //Quarter Step
                          else if(rxBuf[1] == STP_EIGHTH){digitalWrite(MS1, HIGH); digitalWrite(MS2, HIGH); digitalWrite(MS3, LOW);}  //Eighth Step
                          else if(rxBuf[1] == STP_SIXTEENTH){digitalWrite(MS1, HIGH); digitalWrite(MS2, HIGH); digitalWrite(MS3, HIGH);} //Sixteenth Step
                          break;
                
        case STP_RUN_CTRL: stepperRunCtrl = rxBuf[1]; break;
                          
        case STP_ACC_L1: *((char *)(&stepperAcceleration))   = rxBuf[1]; break;
        case STP_ACC_L2: *((char *)(&stepperAcceleration)+1) = rxBuf[1]; break;
        case STP_ACC_H1: *((char *)(&stepperAcceleration)+2) = rxBuf[1]; break;
        case STP_ACC_H2: *((char *)(&stepperAcceleration)+3) = rxBuf[1]; stepper.setAcceleration(stepperAcceleration); break;
        
        case STP_MAX_SPEED_L1: *((char *)(&stepperMaxSpeed))   = rxBuf[1]; break;
        case STP_MAX_SPEED_L2: *((char *)(&stepperMaxSpeed)+1) = rxBuf[1]; break;
        case STP_MAX_SPEED_H1: *((char *)(&stepperMaxSpeed)+2) = rxBuf[1]; break;
        case STP_MAX_SPEED_H2: *((char *)(&stepperMaxSpeed)+3) = rxBuf[1]; stepper.setMaxSpeed(stepperMaxSpeed); break;
                              
        case STP_SPEED_L1: *((char *)(&stepperSpeed))   = rxBuf[1]; break;
        case STP_SPEED_L2: *((char *)(&stepperSpeed)+1) = rxBuf[1]; break;
        case STP_SPEED_H1: *((char *)(&stepperSpeed)+2) = rxBuf[1]; break;
        case STP_SPEED_H2: *((char *)(&stepperSpeed)+3) = rxBuf[1]; stepper.setSpeed(stepperSpeed); break;
        
        case STP_MOVE_TO_L1: *((char *)(&stepperMoveTo))   = rxBuf[1]; break;
        case STP_MOVE_TO_L2: *((char *)(&stepperMoveTo)+1) = rxBuf[1]; break;
        case STP_MOVE_TO_H1: *((char *)(&stepperMoveTo)+2) = rxBuf[1]; break;
        case STP_MOVE_TO_H2: *((char *)(&stepperMoveTo)+3) = rxBuf[1]; stepper.moveTo(stepperMoveTo); break;
                            
        case STP_MOVE_L1: *((char *)(&stepperMove))   = rxBuf[1]; break;
        case STP_MOVE_L2: *((char *)(&stepperMove)+1) = rxBuf[1]; break;
        case STP_MOVE_H1: *((char *)(&stepperMove)+2) = rxBuf[1]; break;
        case STP_MOVE_H2: *((char *)(&stepperMove)+3) = rxBuf[1]; stepper.move(stepperMove); break;
                         
        case STP_CURRENT_POS_L1: *((char *)(&stepperCuttentPos))   = rxBuf[1]; break;
        case STP_CURRENT_POS_L2: *((char *)(&stepperCuttentPos)+1) = rxBuf[1]; break;
        case STP_CURRENT_POS_H1: *((char *)(&stepperCuttentPos)+2) = rxBuf[1]; break;
        case STP_CURRENT_POS_H2: *((char *)(&stepperCuttentPos)+3) = rxBuf[1]; stepper.setCurrentPosition(stepperCuttentPos); break;
        
        default: break;
      }   
      readCount = 0;
      i = 0;
    }
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{
  switch(rxBuf[0])
  {
    case STP_RUN_STATE: Wire.write(stepperRunState); break;

    case STP_SPEED_RL1: stepperSpeedRead = stepper.speed();
                       Wire.write(*((char *)(&stepperSpeedRead))); break;
    case STP_SPEED_RL2: //stepperSpeedRead = stepper.speed();
                       Wire.write(*((char *)(&stepperSpeedRead) + 1)); break;
    case STP_SPEED_RH1: //stepperSpeedRead = stepper.speed();
                       Wire.write(*((char *)(&stepperSpeedRead) + 2)); break;
    case STP_SPEED_RH2: //stepperSpeedRead = stepper.speed();
                       Wire.write(*((char *)(&stepperSpeedRead) + 3)); break;
                       
    case STP_DIS_TOGO_RL1: stepperDistanceToGoRead = stepper.distanceToGo();
                          Wire.write(*((char *)(&stepperDistanceToGoRead))); break;
    case STP_DIS_TOGO_RL2: //stepperDistanceToGoRead = stepper.distanceToGo();
                          Wire.write(*((char *)(&stepperDistanceToGoRead) + 1)); break;
    case STP_DIS_TOGO_RH1: //stepperDistanceToGoRead = stepper.distanceToGo();
                          Wire.write(*((char *)(&stepperDistanceToGoRead) + 2)); break;
    case STP_DIS_TOGO_RH2: //stepperDistanceToGoRead = stepper.distanceToGo();
                          Wire.write(*((char *)(&stepperDistanceToGoRead) + 3)); break;
    
    case STP_TARGET_POS_RL1: stepperTargetPositionRead = stepper.targetPosition();
                            Wire.write(*((char *)(&stepperTargetPositionRead))); break;
    case STP_TARGET_POS_RL2: //stepperTargetPositionRead = stepper.targetPosition();
                            Wire.write(*((char *)(&stepperTargetPositionRead) + 1)); break;
    case STP_TARGET_POS_RH1: //stepperTargetPositionRead = stepper.targetPosition();
                            Wire.write(*((char *)(&stepperTargetPositionRead) + 2)); break;
    case STP_TARGET_POS_RH2: //stepperTargetPositionRead = stepper.targetPosition();
                            Wire.write(*((char *)(&stepperTargetPositionRead) + 3)); break;
                            
    case STP_CURRENT_POS_RL1: stepperCurrentPositionRead = stepper.currentPosition();
                             Wire.write(*((char *)(&stepperCurrentPositionRead))); break;
    case STP_CURRENT_POS_RL2: //stepperCurrentPositionRead = stepper.currentPosition();
                             Wire.write(*((char *)(&stepperCurrentPositionRead) + 1)); break;
    case STP_CURRENT_POS_RH1: //stepperCurrentPositionRead = stepper.currentPosition();
                             Wire.write(*((char *)(&stepperCurrentPositionRead) + 2)); break;
    case STP_CURRENT_POS_RH2: //stepperCurrentPositionRead = stepper.currentPosition();
                             Wire.write(*((char *)(&stepperCurrentPositionRead) + 3)); break;
                             
    default: break;
  }
}


