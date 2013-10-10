#include <Wire.h>
#include <Adafruit_NeoPixel.h> // http://github.com/adafruit/Adafruit_NeoPixel

//address table
#define LS_RUN_STATE     0x91
#define LS_CURRENT       0x92
#define LS_GET_PIXEL_R   0x93
#define LS_GET_PIXEL_G   0x94
#define LS_GET_PIXEL_B   0x95
#define LS_GET_PIXEL_NUM 0x96

#define LS_SET_PIXEL_NUM 0x02
#define LS_SET_PIXEL_R   0x03
#define LS_SET_PIXEL_G   0x04
#define LS_SET_PIXEL_B   0x05
#define LS_SET_SPEED      0x06
#define LS_SET_COUNT      0x07
#define LS_SET_IN_SPEED  0x08

#define LS_RUN_CTRL      0x1A
#define LS_LED_COUNT	 0x1B

//data table
#define LS_NO_FLASH      0x00
#define LS_STOP_FLASH    0x00
#define LS_AUTO_FLASH    0x01
#define LS_ONCE_FLASH    0x02
#define LS_CLOSE         0x04
#define LS_RESET         0x05
#define LS_COLOR_LOOP	 0x06
#define LS_INDICATORS    0x07
#define LS_ALL_PIXEL     0xFE

#define MAX_BRI          150
#define MIN_BRI          0

//pin define
//#define S1 2
//#define S2 3
//#define ADC7 A7

int ledCount = 59;
int phyAddress = 5;
byte ledStripRunCtrl = 0;
byte flashSpeed = 0;
byte indicatorsSpeed = 2;
byte setNum = 0, tempRed = 0, tempGreen = 0, tempBlue = 0;
Adafruit_NeoPixel ledStrip = Adafruit_NeoPixel(60);

void setup()
{
//  pinMode(S1, OUTPUT);
//  pinMode(S2, OUTPUT);
//  pinMode(A7, INPUT);
  
  Wire.begin(5);                // join i2c bus with address #5
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
  ledStrip.begin();
  reset_ls();
//  ledStrip.show();
}

void loop()
{
  switch(ledStripRunCtrl)
  {
    case LS_AUTO_FLASH: ledStrip.show(); delay(flashSpeed); break;
    case LS_ONCE_FLASH: ledStrip.show(); ledStripRunCtrl = 0; break;
//    case LS_STOP_FLASH: break;
//    case LS_NO_FLASH: break;
    case LS_COLOR_LOOP: color_loop(); delay(flashSpeed); break;
    case LS_INDICATORS: indicators(setNum,indicatorsSpeed,tempRed,tempGreen,tempBlue); ledStripRunCtrl = 0; break;
    case LS_RESET: reset_ls(); ledStripRunCtrl = 0; break;
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
        case LS_RUN_CTRL: ledStripRunCtrl = rxBuf[1]; break;
        case LS_LED_COUNT: ledCount = rxBuf[1]>60? 59: rxBuf[1]-1;break;
        case LS_SET_PIXEL_R: tempRed   = rxBuf[1]; break;
        case LS_SET_PIXEL_G: tempGreen = rxBuf[1]; break;
        case LS_SET_PIXEL_B: tempBlue  = rxBuf[1]; break;
        case LS_SET_PIXEL_NUM: if(LS_ALL_PIXEL == rxBuf[1])
                                 for(int x = ledCount; x >= 0; x--){ledStrip.setPixelColor(x,tempRed,tempGreen,tempBlue);}
                               else ledStrip.setPixelColor(setNum,tempRed,tempGreen,tempBlue);
                               break;
        case LS_SET_COUNT: setNum = rxBuf[1]-1; break;
        case LS_SET_SPEED: flashSpeed = rxBuf[1]; break;
        case LS_SET_IN_SPEED: indicatorsSpeed = rxBuf[1]; break;
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
/*    case LS_RUN_STATE: Wire.write(stepperRunState);
                        break;

    case LS_GET_PIXEL_R: stepperSpeedRead = stepper.speed();
                       Wire.write(*((char *)(&stepperSpeedRead)));
                       break;
    case LS_GET_PIXEL_G: //stepperSpeedRead = stepper.speed();
                       Wire.write(*((char *)(&stepperSpeedRead) + 1));
                       break;
    case LS_GET_PIXEL_B: //stepperSpeedRead = stepper.speed();
                       Wire.write(*((char *)(&stepperSpeedRead) + 2));
                       break;
*/
    default: break;
  }
}

void reset_ls()
{
  for(int x = ledCount; x >= 0; x--)
  {
    ledStrip.setPixelColor(x,0,0,0);
  }
  ledStrip.show(); 
}
uint8_t pixel_index;
long last_time;
int j = 0, f = 0, k = 0, count;
void color_loop()
{  
  for (uint8_t t = 0; t < ledCount; t++)
  {
    uint8_t red =   64*(1+sin(t/2.0 + j/4.0       ));
    uint8_t green = 64*(1+sin(t/1.0 + f/9.0  + 2.1));
    uint8_t blue =  64*(1+sin(t/3.0 + k/14.0 + 4.2));
    
    uint32_t pix = green;
    pix = (pix << 8) | red;
    pix = (pix << 8) | blue;
    
    ledStrip.setPixelColor(t, pix);
    if ((millis() - last_time > 15) && pixel_index <= ledCount + 1)
    {
      last_time = millis();
      count = ledCount - pixel_index;
      pixel_index++; 
    }
    
    for (int x = count; x >= 0; x--)
      ledStrip.setPixelColor(x, ledStrip.Color(0,0,0));
  }
  ledStrip.show();
  j = j + random(1,2);
  f = f + random(1,2);
  k = k + random(1,2);
}

int lastNum = 0;
void indicators(uint8_t num, uint8_t inSpeed, uint8_t red, uint8_t green, uint8_t blue)
{
  if(lastNum <= num){
    for(int x = lastNum; x <= num; ++x){
        ledStrip.setPixelColor(x,red,green,blue);
        ledStrip.show();
        delay(inSpeed);
    }
  }
  else{
    for(int x = lastNum; x > num; --x){
        ledStrip.setPixelColor(x,0,0,0);
        ledStrip.show();
        delay(inSpeed);
    }
  }
  lastNum = num; 
}
