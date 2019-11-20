#include <FastLED.h>
#define LED_PIN     3
#define NUM_LEDS    140
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const int buzzerPin = 4;

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";

CRGB leds[NUM_LEDS];

boolean horn = false;
int times = 0;

int charLayout[11][14] = {
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //0
    {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1}, //1
    {1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1}, //2
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1}, //3
    {1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1}, //4
    {1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0}, //5
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, //6
    {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}, //7
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //8
    {1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, //9
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  //CLR
} ; 

//does not include the 1 in third score digit and colon and bonus / poss
int offset[] = {0, 14, 30, 44, 62, 76, 92, 112, 126};

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  
  Serial.begin(9600);
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  pinMode(buzzerPin, OUTPUT);
  
  clearScreen();

  //Draw colon
  leds[28] = CRGB(255, 120, 0);
  leds[29] = CRGB(255, 120, 0);
}

void drawChar(int number, int offsetY, String color) {
  for (int i = 0; i <= 13; i++) {
    if(charLayout[number][i] == 1) {//chose num
      if(color == "red") {
        leds[i + offset[offsetY]] = CRGB (220, 0, 0);
      }else if(color == "yellow") {
        leds[i + offset[offsetY]] = CRGB (255, 120, 0);
      }
    }else {
      leds[i + offset[offsetY]] = CRGB (0, 0, 0);
    }
  }
}

void drawOther(String item) {
  if(item == "hOne") {
    for(int i = 58; i <= 61; i++) {
      leds[i] = CRGB(220, 0, 0);
    }
    
  }else if(item == "hClear") {
    for(int i = 58; i <= 61; i++) {
      leds[i] = CRGB(0, 0, 0);
    }
    
  }else if(item == "gOne") {
    for(int i = 108; i <= 111; i++) {
      leds[i] = CRGB(220, 0, 0);
    }
    
  }else if(item == "gClear") {
    for(int i = 108; i <= 111; i++) {
      leds[i] = CRGB(0, 0, 0);
    }
  }
  
}

void clearScreen() {
  for (int i = 0; i <= 139; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void loop() {
  if(times >= 10) {
    horn = false;
    times = 0;
  }else{
    times++;
  }

  Serial.println(times);
  
  if (radio.available()) {
    char clk[32] = "";
    radio.read(&clk, sizeof(clk));

    if(String(clk[0]) == "t") {
      if((clk[1] - 0) - '0' != 0) {
        drawChar((clk[1] - 0) - '0', 0, "yellow");
      }else{
        drawChar(10, 0, "");
      }
      drawChar((clk[2] - 0) - '0', 1, "yellow");
      drawChar((clk[4] - 0) - '0', 2, "yellow");
      drawChar((clk[5] - 0) - '0', 3, "yellow");
      
      FastLED.show();
      
    }

    if(String(clk[0]) == "h") {
      if((clk[1] - 0) - '0' != 0) {
        drawOther("hOne");
      }else{
        drawOther("hClear");
      }
      
      if(((clk[2] - 0) - '0' == 0) && ((clk[1] - 0) - '0' == 0)) {
        drawChar(10, 4, "");
        
      }else{
        drawChar((clk[2] - 0) - '0', 4, "red");
      }
      
      drawChar((clk[3] - 0) - '0', 5, "red");
      
      FastLED.show();
    }

    if(String(clk[0]) == "g") {
      if((clk[1] - 0) - '0' != 0) {
        drawOther("gOne");
      }else{
        drawOther("gClear");
      }
      
      if(((clk[2] - 0) - '0' == 0) && ((clk[1] - 0) - '0' == 0)) {
        drawChar(10, 7, "");
        
      }else{
        drawChar((clk[2] - 0) - '0', 7, "red");
      }
      
      drawChar((clk[3] - 0) - '0', 8, "red");
      
      FastLED.show();
    }

    if(String(clk[0]) == "p") {
      drawChar((clk[1] - 0) - '0', 6, "yellow");
      
      FastLED.show();
    }

    if(String(clk[0]) == "a") { //home bonus
      if((clk[1] - 0) - '0' == 1) {
        leds[90] = CRGB(255, 120, 0);
      }else{
        leds[90] = CRGB(0, 0, 0);
      }

      FastLED.show();
    }

    if(String(clk[0]) == "b") { //guest bonus
      if((clk[1] - 0) - '0' == 1) {
        leds[106] = CRGB(255, 120, 0);
      }else{
        leds[106] = CRGB(0, 0, 0);
      }

      FastLED.show();
    }

    if(String(clk[0]) == "c" ) { //home poss
      if((clk[1] - 0) - '0' == 1) {
        leds[91] = CRGB(220, 0, 0);
      }else{
        leds[91] = CRGB(0, 0, 0);
      }

      FastLED.show();
    }

    if(String(clk[0]) == "d") { //guest poss
      if((clk[1] - 0) - '0' == 1) {
        leds[107] = CRGB(220, 0, 0);
      }else{
        leds[107] = CRGB(0, 0, 0);
      }

      FastLED.show();
    }

    if(String(clk[0]) == "o") {
      horn = true;
      times = 0;
    }else{
      horn = false;
    }
    
  }

  Serial.println(horn);

  if(horn == true) {
    digitalWrite(buzzerPin, HIGH);
  }else{
    digitalWrite(buzzerPin, LOW);
  }
}
