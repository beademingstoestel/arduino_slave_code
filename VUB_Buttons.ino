#include <Arduino.h>
#include <HardwareSerial.h>
#include <Print.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "pins.h"



//Constants Tim


LiquidCrystal_I2C lcd(0x3f, 20,  4);


// Communication with Main
String value0;
String value1;
const byte numChars = 32;
char receivedChars0[numChars];
char receivedChars1[numChars];
boolean newData0 = false;
boolean newData1 = false;



//Constants Hans

// define bytes to send
unsigned int RR = 0;    // Number of breaths per minute setting
unsigned int VT = 0;    // Tidal volume= target to deliver
unsigned int PK = 50;   //Peak pressure
unsigned int TS = 0;    // Breath Trigger Sensitivity = amount the machine should look for
float IE = 0;           // Inspiration-expiration rate
// unsigned int PP = 0;    // PEEP Pressure = Max pressure to deliver --> Manueel instellen op peep valve

unsigned int ADPK = 10; // Allowed Deviation Peak pressure
unsigned int ADVT = 10; // Allowed Deviation Tidal Volume
unsigned int ADPP = 5;  // Allowed Deviation PEEP

bool MODE = false; // Mode (false = Pressure)
bool MUTE = false; // Mute alarms (true = mute alarms)
bool ACTIVE = false; // Start/stop (true = start)

bool flag = false;


// constants won't change. They're used here to set pin numbers:
int buttonState;             // the current reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers
uint32_t lastRequestedValueTime = 0;
uint32_t loopMillis;

const int numButtons = 20;
unsigned long lastDebounceTime[2][numButtons] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}; // the last time the output pin was toggled
//make an array with all button pins and last button state
int buttonPins[2][numButtons] = { {
    BUTTON_VOLUME_DOWN,
    BUTTON_VOLUME_UP,
    BUTTON_PRESSURE_DOWN,
    BUTTON_PRESSURE_UP,
    BUTTON_TRIG_DOWN,
    BUTTON_TRIG_UP,
    BUTTON_RR_DOWN,
    BUTTON_RR_UP,
    BUTTON_START_STOP,
    BUTTON_MUTE,
    BUTTON_HOLD,
    BUTTON_MODE,
    BUTTON_PRESSURE_ALARM_DOWN,
    BUTTON_PRESSURE_ALARM_UP,
    BUTTON_VOLUME_ALARM_DOWN,
    BUTTON_VOLUME_ALARM_UP,
    BUTTON_PEEP_ALARM_DOWN,
    BUTTON_PEEP_ALARM_UP,
    BUTTON_IE_DOWN,
    BUTTON_IE_UP
  }, {
    HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH
  }
};
int buttons [numButtons] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Constants Loes
#define POS_VOLUME_DOWN     0
#define POS_VOLUME_UP       1
#define POS_PRESSURE_DOWN   2
#define POS_PRESSURE_UP     3
#define POS_TRIG_DOWN       4
#define POS_TRIG_UP         5
#define POS_RR_DOWN        6
#define POS_RR_UP          7
#define POS_START_STOP      8
#define POS_MUTE            9
#define POS_HOLD            10
#define POS_MODE            11
#define POS_PRESSURE_ALARM_DOWN 12
#define POS_PRESSURE_ALARM_UP   13
#define POS_VOLUME_ALARM_DOWN   14
#define POS_VOLUME_ALARM_UP     15
#define POS_PEEP_ALARM_DOWN     16
#define POS_PEEP_ALARM_UP       17
#define POS_IE_DOWN             18
#define POS_IE_UP               19


bool test = false;

void setup() {
  //for debugging
  Serial.begin(115200);
   Serial.println("Init LCD");
  //init LCD
  lcd.backlight();
  lcd.init();
  //print values void
   Serial.println("Print letters");
  printLetters();

 /*Serial.println("Wacht op OK");
  while(newData1 == 0 ){
      recvWithEndMarkerSer1();
      if (newData1 == true){
        //processSerialPort(receivedChars1);
        if (receivedChars1 == 'OK'){
          Serial.println("Joepie!");
        } else {
          newData1 = false;
        }
        
      }
      for (int i = 0; i < numButtons; i++) {
        buttons[i]=1;
      }
      serialSend();
  }
*/


  
  //declare all buttons
  pinMode(BUTTON_VOLUME_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_UP, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_UP, INPUT_PULLUP);
  pinMode(BUTTON_TRIG_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_TRIG_UP, INPUT_PULLUP);
  pinMode(BUTTON_RR_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_RR_UP, INPUT_PULLUP);
  pinMode(BUTTON_START_STOP, INPUT_PULLUP);
  pinMode(BUTTON_MUTE, INPUT_PULLUP);
  pinMode(BUTTON_HOLD, INPUT_PULLUP);
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_ALARM_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PRESSURE_ALARM_UP, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_ALARM_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_VOLUME_ALARM_UP, INPUT_PULLUP);
  pinMode(BUTTON_PEEP_ALARM_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PEEP_ALARM_UP, INPUT_PULLUP);
  pinMode(BUTTON_IE_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_IE_UP, INPUT_PULLUP);
}

void loop() {

  loopMillis = millis();
  //read button inputs
  buttonsRead();
  //array with buttons is filled with data

  //edit parameters

  //send parameters
  if (flag == true) {
    serialSend();
    flag = false;
  }
  delay(10);
  
  //Update screen
  printValues();


  delay(100);

}


void buttonsRead() {
  //read out the buttons with debounce
  for (int i = 0; i < numButtons; i++)
  {
    //sample the state of the button - is it pressed or not?
    int  reading = digitalRead(buttonPins[0][i]);
    //filter out any noise by setting a time buffer
    if ( (millis() - lastDebounceTime[0][i]) > debounceDelay) {
      if ( reading == HIGH && buttonPins[1][i] != HIGH) {
        buttonPins[1][i] = reading;
        lastDebounceTime[0][i] = millis(); //set the current time
        lastDebounceTime[1][i] = 0; //reset the first time counter for jumpvalue
      }
      else if ( reading == LOW && buttonPins[1][i] != LOW) {
        buttonPins[1][i] = reading;
        lastDebounceTime[0][i] = millis(); //set the current time

        if (lastDebounceTime[1][i] == 0)
        {
          lastDebounceTime[1][i] = millis(); //set the current time to the first time counter for jumpvalue
          flag = true;
        }

      }//close if/else
    }//close if(time buffer)
  }

  //change the sign of the buttons
  for (int j = 0; j < numButtons; j++)
  {
    if (buttonPins[1][j] == 0)
    {
      buttons[j] = 1;
    }
    else
    {
      buttons[j] = 0;
    }
  }
  
  //make the buttons edit the values
  //RR
  RR = RR +  buttons[POS_RR_UP] * jumpValue(POS_RR_UP);
  RR = RR -  buttons[POS_RR_DOWN] * jumpValue(POS_RR_DOWN);
  // VT
  VT = VT +  buttons[POS_VOLUME_UP] * jumpValue(POS_VOLUME_UP);
  VT = VT -  buttons[POS_VOLUME_DOWN] * jumpValue(POS_VOLUME_DOWN);
  // PK
  PK = PK +  buttons[POS_PRESSURE_UP] * jumpValue(POS_PRESSURE_UP);
  PK = PK -  buttons[POS_PRESSURE_DOWN] * jumpValue(POS_PRESSURE_DOWN);
  //TS
  TS = TS +  buttons[POS_TRIG_UP] * jumpValue(POS_TRIG_UP);
  TS = TS -  buttons[POS_TRIG_DOWN] * jumpValue(POS_TRIG_DOWN);
  //IE
  IE = IE + 0.1 * buttons[POS_IE_UP];
  IE = IE - 0.1 * buttons[POS_IE_DOWN];

  // ADPK
  ADPK = ADPK +  buttons[POS_PRESSURE_ALARM_UP] * jumpValue(POS_PRESSURE_ALARM_UP);
  ADPK = ADPK -  buttons[POS_PRESSURE_ALARM_DOWN] * jumpValue(POS_PRESSURE_ALARM_DOWN);
  // ADVT
  ADVT = ADVT +  buttons[POS_VOLUME_ALARM_UP] * jumpValue(POS_VOLUME_ALARM_UP);
  ADVT = ADVT -  buttons[POS_VOLUME_ALARM_DOWN] * jumpValue(POS_VOLUME_ALARM_DOWN);
  // ADPP
  ADPP = ADPP +  buttons[POS_PEEP_ALARM_UP] * jumpValue(POS_PEEP_ALARM_UP);
  ADPP = ADPP -  buttons[POS_PEEP_ALARM_DOWN] * jumpValue(POS_PEEP_ALARM_DOWN);
  
  //mode button here
  //if mode is VOLUME and button is pressed: goto PRESSURE
  if (buttons[POS_MODE] == 1)
  {
    MODE = !MODE;
    clearValues();
  }


  if (buttons[POS_MUTE] == 1)
  {
    MUTE = !MUTE;
    clearValues();
  }


  // TODO: hold, mute, start/stop
  

}


int jumpValue(int i) {

  //  if (loopMillis - lastRequestedValueTime < 200) {
  //    return 0;
  // }
  //  lastRequestedValueTime = loopMillis;
  //Serial.println(loopMillis);
  //Serial.println(lastDebounceTime[1][i]);
  // Serial.println(loopMillis-lastDebounceTime[1][0]);



  if ( loopMillis - lastDebounceTime[1][i] > 4000) {
    return 10;
  }
  if ( loopMillis - lastDebounceTime[1][i] > 2000) {
    return 5;
  }
  if (loopMillis - lastDebounceTime[1][i] > 1000) {
    return 3;
  }
  if (loopMillis - lastDebounceTime[1][i] > 500) {
    return 2;
  }
  return 1;
}

void serialSend()
{
  // RR
  if ((buttons[POS_RR_UP] == 1) || (buttons[POS_RR_DOWN] == 1)) {
    delay(10);
    Serial.print("RR=");
    Serial.println(RR);
  }
  // VT
  if ((buttons[POS_VOLUME_UP] == 1) || (buttons[POS_VOLUME_DOWN] == 1)) {
    delay(10);
    Serial.print("VT=");
    Serial.println(VT);
  }
  // PK
  if ((buttons[POS_PRESSURE_UP] == 1) || (buttons[POS_PRESSURE_DOWN] == 1)) {
    delay(10);
    Serial.print("PK=");
    Serial.println(PK);
  }
  // TS
  if ((buttons[POS_TRIG_UP] == 1) || (buttons[POS_TRIG_DOWN] == 1)) {
    delay(10);
    Serial.print("TS=");
    Serial.println(TS);
  }
  // IE
  if ((buttons[POS_IE_UP] == 1) || (buttons[POS_IE_DOWN] == 1)) {
    delay(10);
    Serial.print("IE=");
    Serial.println(IE);
  }
  // PP --> kunnen we niet doorsturen

  // ADPK
  if ((buttons[POS_PRESSURE_ALARM_UP] == 1) || (buttons[POS_PRESSURE_ALARM_DOWN] == 1)) {
    delay(10);
    Serial.print("ADPK=");
    Serial.println(ADPK);
  }
  // ADVT
  if ((buttons[POS_VOLUME_ALARM_UP] == 1) || (buttons[POS_VOLUME_ALARM_DOWN] == 1)) {
    delay(10);
    Serial.print("ADVT=");
    Serial.println(ADVT);
  }
  // ADPP
  if ((buttons[POS_PEEP_ALARM_UP] == 1) || (buttons[POS_PEEP_ALARM_DOWN] == 1)) {
    delay(10);
    Serial.print("ADPP=");
    Serial.println(ADPP);
  }
    
  // MODE
  if (buttons[POS_MODE] == 1) {
    Serial.print("MODE=");
    Serial.println(MODE);
  }

  // MUTE
  if (buttons[POS_MUTE] == 1) {
    Serial.print("MUTE=");
    Serial.println(MUTE);
  }
  
  // ACTIVE
  if (buttons[POS_START_STOP] == 1) {
    Serial.print("ACTIVE=");
    Serial.println(ACTIVE);
  }
  // TODO: hold
}

/*void serialSendSpecific(){
  Serial.print();
}*/

void printValues() {
  lcd.setCursor(12, 2);
  if (MODE == true) lcd.print("VOLUME");
  if (MODE == false) lcd.print("PRESSURE");
  lcd.setCursor(3, 0);
  lcd.print(PK);
  lcd.setCursor(7, 0);
  lcd.print(ADPK);
  lcd.setCursor(3, 1);
  lcd.print(VT);
  lcd.setCursor(7, 1);
  lcd.print(ADVT);
  lcd.setCursor(15, 0);
  lcd.print(RR);
  lcd.setCursor(15, 1);
  lcd.print(TS);
//  lcd.setCursor(3, 2);
//  lcd.print(PP);
  lcd.setCursor(7, 2);
  lcd.print(ADPP);
  lcd.setCursor(15, 3);
  lcd.print(IE);
}
void printLetters()
{
  lcd.setCursor(0, 0);
  lcd.print("PK");
  lcd.setCursor(0, 1);
  lcd.print("VT");
  lcd.setCursor(12, 0);
  lcd.print("RR");
  lcd.setCursor(12, 1);
  lcd.print("TS");
//  lcd.setCursor(0, 2);
//  lcd.print("PP");
  lcd.setCursor(12, 3);
  lcd.print("IE");
}

void clearValues()
{
  lcd.setCursor(12, 2);
  lcd.print("        ");
  lcd.setCursor(3, 0);
  lcd.print("   ");
  lcd.setCursor(7, 0);
  lcd.print("   ");
  lcd.setCursor(3, 1);
  lcd.print("   ");
  lcd.setCursor(7, 1);
  lcd.print("   ");
  lcd.setCursor(15, 0);
  lcd.print("   ");
  lcd.setCursor(15, 1);
  lcd.print("   ");
  lcd.setCursor(3, 2);
  lcd.print("   ");
  lcd.setCursor(7, 2);
  lcd.print("   ");
  lcd.setCursor(15, 3);
  lcd.print("   ");
}

void recvWithEndMarkerSer1() {
   static byte ndx = 0;
   char endMarker = '\n';
   char rc;
   while (Serial.available() > 0 && newData1 == false) {
     rc = Serial.read();
     if (rc != endMarker) {
       receivedChars1[ndx] = rc;
       ndx++;
       if (ndx >= numChars) {
         ndx = numChars - 1;
       }
     }
     else {
       receivedChars1[ndx] = '\0'; // terminate the string
       ndx = 0;
       newData1 = true;
       Serial.println(receivedChars1);
     }
   }
}
