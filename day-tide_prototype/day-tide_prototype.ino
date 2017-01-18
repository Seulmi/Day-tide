/*************************************************************
   Title:
   Create:
   Update: 2016.10.29
   Version: a1
   About:
   Settings:
   -- DFPlayer.rx --> Arduino.d11
   -- DFPlayer.tx --> Arduino.d10
   -- neoPixel    --> Arduino.d06
   -- push button --> Arduino.d08 (red button, red leds)
   -- tilt switch --> Arduino.d05 (plays music, no loop)
   -- push button --> Arduino.d02 (white leds, plays music #2, loops)
 *************************************************************/
/*************************************************************
   Program Flow Vars Start
 *************************************************************/
boolean debug = true;
int state = 0;
int prevSwitchValue, prevTactValue;
/*
   state 0 --> 초기/대기 상태
   state 1 --> 바로 세운 상태에서 red btn을 누른 상태
   state 2 --> 상대방이 보내준 사운드를 1회만 재생하는 상태
   state 3 --> 상대방이 보내준 사운드 재생(무한) 하는 상태
*/
/*************************************************************
   Program Flow Vars End
 *************************************************************/
/*************************************************************
   DFPlayer Vars Start
 *************************************************************/
#include <DFPlayer_Mini_Mp3.h>
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); //d10 --> DFPlayer.rx, d11 --> DFPlater.tx
int playerIsBusyPin = 3;                 //d03 --> DFPlayer.busy
int playerIsBusyValue;
int currentSound = 3;
/*
  boolean played = false;
  boolean ablestop = false;
  boolean musicloop = false;
  boolean tactpressed = false;
*/
/*************************************************************
   DFPlayer Vars End
 *************************************************************/
/*************************************************************
   Neo Plxel Vars Start
 *************************************************************/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
//#define NUM_LEDS 60
#define BRIGHTNESS 40

// How many NeoPixels we will be using, charge accordingly
const int neoPixelPin = 6;    // d06 --> Neo Pixel
const int switchPin = 8;      //red button
const int tiltPin = 5;
const int tactPin = 2;        //small btn

int neoPixelValue, switchValue, tiltValue, tactValue;

int numPixels = 25;
/*
  int buttonPushCounter = 0;   // counter for the number of button presses
  int buttonState = 0;         // current state of the button
  int tactState = 0;           // current state of the tact swtich
  int lastbuttonState = 0;     // previous state of the button

  boolean redbutton = false;
  boolean whitebutton = false;
  boolean whitelong = false;
  int var = 0;
*/
// Instatiate the NeoPixel from the ibrary
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, neoPixelPin, NEO_GRBW + NEO_KHZ800);

int gamma[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};
/*************************************************************
   Neo Plxel Vars End
 *************************************************************/
/*************************************************************
   setup() Start
 *************************************************************/
void setup () {
  Serial.begin (9600);
  mySerial.begin (9600);
  mp3_set_serial (mySerial);    // set DFPlayer serial pins

  pinMode(switchPin, INPUT);       // red btn
  pinMode(tactPin, INPUT);         // small btn
  pinMode(neoPixelPin, OUTPUT);    // neo pixel
  pinMode(tiltPin, INPUT);         // tile switch
  pinMode(playerIsBusyPin, INPUT); // dfplayer is busy

  strip.begin();  // initialize the strip
  strip.show();   // make sure it is visible
  strip.clear();  // Initialize all pixels to 'off'

  delay(1000);
  //setVolume(0);
}
/*************************************************************
   setup() End
 *************************************************************/
/*************************************************************
   loop() Start
 *************************************************************/
void loop () {
  /////////////////////////////////////////////////////////////
  // Debug
  /////////////////////////////////////////////////////////////
  if (Serial.available() > 0) {
    char c = Serial.read();
    Serial.println(c);
    state = c - '0';
    Serial.print("State:");
    Serial.println(state - '0');
  }
  if (debug) {

  }
  /////////////////////////////////////////////////////////////
  // Read Sensors
  /////////////////////////////////////////////////////////////
  readSensors();
  if (debug) printSensors();
  /*
     switchPin       --> switchValue
     tactPin         --> tactValue
     neoPixelPin     --> neoPixelValue
     tiltPin         --> tileValue
     playerIsBusyPin --> playerIsBusyValue
  */
  /////////////////////////////////////////////////////////////
  // Process control
  /////////////////////////////////////////////////////////////
  //Serial.println(switchValue);
  //Serial.println(digitalRead(tiltPin));
  if (tiltValue == LOW) {
    if (switchValue == HIGH && prevSwitchValue == LOW && (state <= 10)) {
      state = 20;                       // State02: neo pixel을 red color로 설정하고 회전시키기
      delay(300);
    } else if (switchValue == HIGH && prevSwitchValue == LOW && (state == 20)) {
      delay(300);
      state = 29;                       // State02: 종료
    } else if (state > 30) {
      state = 10;                       // State01: 기본(초기) 상태로 복귀
    }
  } else if (tiltValue == HIGH) {
    if (state == 0 || state == 10) {
      delay(200);
      state = 30;                       // State03: 상대방이 보내준 사운드 재생(1회)
    } else if (tactValue == HIGH && prevTactValue == LOW && (state == 39 || state <= 10)) {
      delay(200);
      state = 40;                       // State04: 상대방이 보내준 사운드 재생(무한)
      currentSound = 5;
    }
  }
  prevSwitchValue = switchValue;
  prevTactValue = tactValue;

  /////////////////////////////////////////////////////////////
  // State: 0 -- 모든 변수/상태 초기화
  /////////////////////////////////////////////////////////////
  if (state == 0) {
    initAll();
    printState();
    //state = -1;
    pause();
  }

  /////////////////////////////////////////////////////////////
  // State: 1 -- 바로 세운 상태(tileValue == 0)이며 아무런 버튼도 누르지 않은 상태
  /////////////////////////////////////////////////////////////
  if (state == 10) {
    standBy();
    printState();
    stopMP3();
    pause();
    //state = -1;
  }

  /////////////////////////////////////////////////////////////
  // State: 2 -- red btn을 눌러서 red led를 작동시키는 상태
  /////////////////////////////////////////////////////////////
  if (state == 20) {                           // neo pixel을 red, 50 delay를 주고 wipe
    pause();
    colorWipe(strip.Color(255, 0, 0), 50);
    strip.clear();  // neo pixel을 끔//strip.clear();  // neo pixel을 끔
    printState();
    Serial.println("state 20");
  } else if (state == 29) {                    // neo pixel을 끔
    pause();
    for ( int i = 0; i < numPixels; i++ ) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
    state = 0;
  }
  /////////////////////////////////////////////////////////////
  // State: 3 -- 상대방이 보내준 사운드 재생(1회), white led FI/FO
  /////////////////////////////////////////////////////////////
  if (state == 30) {
    setVolume(60);
    specify(3);            
    //specify(current);             //music 4

    play();
    pulseWhite(5);                          // neo pixel을 white, 5 delay를 주고 wipe
    printState();
    state = 38;
  } else if (state == 38) {
    pulseWhite(5);                          // neo pixel을 white, 5 delay를 주고 wipe
    printState();
    if (playerIsBusyValue == 1) {  //1 = player idle
      state = 39;
    }
  }

  /////////////////////////////////////////////////////////////
  // State: 4 -- 상대방이 보내준 사운드 재생(1회), white led FI/FO
  /////////////////////////////////////////////////////////////
  if (state == 40) {
    printState();
    setVolume(60);
    specify(1);             //music 4
    play();
    pulseWhite(5);                          // neo pixel을 red, 50 delay를 주고 wipe
    state = 49;

  } else if (state == 49) {
    printState();
    pulseWhite(5);                          // neo pixel을 red, 50 delay를 주고 wipe
    if (playerIsBusyValue == 1) {  //1 = player idle
      // state 40에서의 음악(currentSound = 4)을 루프시킴
      state = 40;
      //currentSound++;
      if (currentSound > 3) {
        currentSound = 1;
      }
      Serial.print("currentSound:");
      Serial.println(currentSound);
    }
  }

}
/*************************************************************
   loop() End
 *************************************************************/
/*************************************************************
   Functions Start
 *************************************************************/
void readSensors() {
  switchValue = digitalRead(switchPin);             // red button
  tactValue = digitalRead(tactPin);                 //
  tiltValue = digitalRead(tiltPin);
  playerIsBusyValue = digitalRead(playerIsBusyPin);
}
void printSensors() {
  Serial.print(state);
  Serial.print("\tswitchValue:");
  Serial.print(switchValue);
  Serial.print("\ttactValue:");
  Serial.print(tactValue);
  Serial.print("\ttiltValue:");
  Serial.print(tiltValue);
  Serial.print("\tplayerIsBusyValue:");
  Serial.println(playerIsBusyValue);
}
void initAll() {
  //state = 0;
}
void standBy() {

}

void printState() {
  Serial.print("state:");
  Serial.print(state);
  Serial.print(", ");
  Serial.println(int(state));
}
/*************************************************************
   Functions End
 *************************************************************/
/*************************************************************
   Neo Plxel Functions Start
 *************************************************************/
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
    //if (debug) Serial.println("colorWipe");
  }
}

void pulseWhite(uint8_t wait) {
  for (int j = 0; j < 256 ; j++) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0, gamma[j] ) );
      //strip.show();
      //delay(wait);

    }
    strip.show();
    delay(wait);
  }
  for (int j = 255; j >= 0 ; j--) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0, gamma[j] ) );
    }
    strip.show();
    delay(wait);
  }
  strip.show();
}
/*************************************************************
   Neo Plxel Functions End
 *************************************************************/
/*************************************************************
   DFPlayer Functions Start
 *************************************************************/
void play()
{
  execute_CMD(0x0D, 0, 1);
  delay(500);
}

void specify(int track)
{
  execute_CMD(0x03, 0, track);
  delay(500);
  // execute_CMD(0x11, 0, 1);
  delay(500);
}


void pause()
{
  execute_CMD(0x0E, 0, 0);
  delay(500);
  execute_CMD(0x16, 0, 0);
  delay(500);
}

void stopMP3() {

}

void mp3_get_state ();


void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}


void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
  // Calculate the checksum (2 bytes)
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                            Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte
                          };
  //Send the command line to the module
  for (byte k = 0; k < 10; k++)
  {
    mySerial.write( Command_line[k]);
  }
}
/*************************************************************
   DFPlayer Functions End
 *************************************************************/
/*************************************************************
   End Of Sketch
 *************************************************************/
