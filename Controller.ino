#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(49, 47); // CE, CSN
const byte address[6] = "00001";

LiquidCrystal lcd(22, 23, 24, 25, 26, 27);

const int timeToggle = 42;
const int buzzer = 36;
const int hornBtn = 44;
const byte ROWS = 4;
const byte COLS = 7;

char keys [ROWS] [COLS] = {
  {'a', 'b', 'c',    's', '7', '8', '9'},
  {'d', 'i', 'j',    't', '4', '5', '6'},
  {'e', 'f', 'g',    '+', '1', '2', '3'},
  {'h', 'k', 'l',    'p', 'y', '0', 'n'}
};
byte rowPins[ROWS] = {5 ,4, 3, 2};
byte colPins[COLS] = {13, 12, 11, 10, 9, 8, 7};

unsigned long currentTime;
unsigned long previousTime;

unsigned long currentHorn;
unsigned long previousHorn;

int buzzTime;
int hornTime;
int oldTime;
int interval=100;

String gScore; // to figure out where G: sits on hs
int oldHScore, oldGScore, oldPeriod;
int homeScore = 0;
int guestScore = 0;
int period = 1;
boolean homePoss = false;
boolean guestPoss = false;
boolean homeBonus = false;
boolean guestBonus = false;

boolean timer, hscore, gscore, periods, ct;

boolean horn = false;
boolean hhorn = false;
int numLength = 0;
String num1="";
String num2="";
String num3="";
String num4="";
String num5="";
String num6="";
String num7="";
String M1, M2, S1, S2, s;
int m1,m2,s1,s2,S;

int buttonState = 0;
int hornState = 0;

boolean selected = false;
boolean pressed = false;
boolean updated = false;
boolean used = false;
boolean on = false;
boolean down = true;

Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  lcd.begin(16,2);
  lcd.clear();

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  
  pinMode(timeToggle, INPUT);
  pinMode(hornBtn, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(buzzer, HIGH);
  digitalWrite(buzzer, LOW);

  lcd.setCursor(2,0);
  lcd.print("Timetronics");
  lcd.setCursor(2,1);
  lcd.print("Scoreboards");
  delay(2000);
  lcd.clear();

  M1="1";
  M2="5";
  S1="0";
  S2="0";
  s="0";

  sendData("time");
  sendData("hScore");
  sendData("gScore");
  sendData("period");
  sendData("other");

  digitalWrite(LED_BUILTIN, HIGH);

  updated=true;
  hhorn = false;
  homeScreen();

  Serial.begin(9600);
}

void homeScreen() {
  pressed=false;
  gScore = guestScore;
  int numLength = gScore.length();
  lcd.setCursor(0,0);
  lcd.print(M1);
  lcd.print(M2);
  lcd.print(":");
  lcd.print(S1);
  lcd.print(S2);
  lcd.print(".");
  if(s != "-1") {
    lcd.print(s);
  }

  lcd.setCursor(13,0);
  lcd.print("P:");
  lcd.setCursor(15,0);
  lcd.print(period);

  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.setCursor(2,1);
  lcd.print(homeScore);
  lcd.setCursor(11,1);
  lcd.print("G:");
  lcd.setCursor(13,1);
  lcd.print(guestScore);

  if(homePoss==true) {
    lcd.setCursor(7,1);
    lcd.print("<");
  }else if(guestPoss==true) {
    lcd.setCursor(8,1);
    lcd.print(">");
  }
  if(homeBonus==true) {
    lcd.setCursor(6,1);
    lcd.print("B");
  }
  if(guestBonus==true) {
    lcd.setCursor(9,1);
    lcd.print("B");
  }
}

void loop() {
  char key = myKeypad.getKey();
  buttonState=digitalRead(timeToggle);
  hornState=digitalRead(hornBtn);

  //mini buzzer for controller
  if(key!=NO_KEY && key!='o') {
    if(buzzTime=1500) {
      digitalWrite(buzzer,HIGH) ;
      buzzTime=0;
    }else{
      buzzTime++;
    }
  }else{
    digitalWrite(buzzer,LOW);
  }

//TIME
  m1=M1.toInt();
  m2=M2.toInt();
  s1=S1.toInt();
  s2=S2.toInt();
  S=s.toInt();

  if(buttonState==LOW) {
    //Send time data
    
    if(down==true) {
      if(m1==0 && m2==0 && s1==0 && s2==0 && S==0) {
        if(hornTime >= 40 && hhorn == false) {
          hornTime=0;
          hhorn = true;

          lcd.setCursor(7, 0);
          lcd.print(" ");
          
        }else{
          hornTime++;

          sendData("horn");
          digitalWrite(LED_BUILTIN, HIGH);
          
          //lcd.setCursor(7, 0);
          //lcd.print("*");
        }
        
      }else{
        if(updated == true) {
          //lcd.setCursor(7,0);
          //lcd.print(" ");
        }
        if(S<0) {
          S=9;
          s2=s2-1;
          s=String(S);
          S2=String(s2);
        }if(s2<0) {
          s2=9;
          s1=s1-1;
          S2=String(s2);
          S1=String(s1);
        }if(s1<0) {
          s1=5;
          m2=m2-1;
          S1=String(s1);
          M2=String(m2);

          //Resend all data every 10 seconds
          sendData("time");
          sendData("hScore");
          sendData("gScore");
          sendData("period");
          sendData("other");
          digitalWrite(LED_BUILTIN, HIGH);
        }if(m2<0) {
          m2=9;
          m1=m1-1;
          M1=String(m1);
          M2=String(m2);
        }if(m1<0) {
          m1=0;
          M1=String(m1);
        }
        currentTime=millis();
        if(currentTime-previousTime>=94) {
          S=S-1;
          previousTime=millis();

          //For testing purposes
          //Serial.println(S);

          sendData("time");
          digitalWrite(LED_BUILTIN, HIGH);
        }
      }
    }else if(down==false) {
      if(m1==9 && m2==9 && s1==9 && s2==9 && S==9) {
        S=0;
        s2=0;
        s1=0;
        m2=0;
        m1=0;
      }else{
        if(S<9) {
          if(updated==true) {
            lcd.setCursor(7,0);
            lcd.print(" ");
          }
          S=0;
          s2++;
          s=String(S);
          S2=String(s2);
        }if(s2<9) {
          s2=0;
          s1++;
          S2=String(s2);
          S1=String(s1);
        }if(s1<5) {
          s1=0;
          m2++;
          S1=String(s1);
          M2=String(m2);
        }if(m2<9) {
          m2=0;
          m1++;
          M1=String(m1);
          M2=String(m2);
        }if(m1<9) {
          m1=0;
          M1=String(m1);
        }
        currentTime=millis();
        if(currentTime-previousTime>=95) {
          S++;
          previousTime=millis();

          sendData("time");
          digitalWrite(LED_BUILTIN, HIGH);
        }
      }
    }
}
    
  M1=String(m1);
  M2=String(m2);
  S1=String(s1);
  S2=String(s2);
  s=String(S);

  //HOME
  if(key=='t' && pressed==false) {
    updated = true;
    lcd.clear();
    homeScreen();
  }

  if(updated==true) {
    homeScreen();
  }

  //SET

   if(key=='s'){
    updated = false;
    pressed = true;
    selected = true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set...");
  }

  //time
  if(key=='t' && pressed==true && selected == true){
    if(buttonState == LOW) {
      selected = false;
      lcd.setCursor(0,0);
      lcd.print("Stop clock");
      lcd.setCursor(0,1);
      lcd.print("before using set");
      pressed=false;
      periods=false;
      hscore=false;
      gscore=false;
      timer=false;
      updated=true;
      ct=false;
      used=false;
      lcd.clear();
      homeScreen();
      
    }else {
      selected = false;
      num4="";
      num7="";
  
      M1="";
      M2="";
      S1="";
      S2="";
      s="";
      timer=true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set Time:");
      lcd.setCursor(0,1);
      lcd.print("MM:SS.s");
    }
    
  }else if(key=='a' && pressed==true && selected == true) {
    selected = false;
    num1 = "";
    hscore=true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set Home Score:");
    lcd.setCursor(0,1);
    lcd.print(homeScore);

    sendData("hScore");
    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if(key=='e' && pressed==true && selected == true) {
    selected = false;
    num2 = "";
    gscore=true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set Guest Score:");
    lcd.setCursor(0,1);
    lcd.print(guestScore);

    sendData("gScore");
    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if(key=='p' && pressed==true && selected == true) {
    selected = false;
    num3 = "";
    periods=true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set Period:");
    lcd.setCursor(0,1);
    lcd.print(period);

    sendData("period");
    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if(key=='+' && pressed==true && selected == true) {
    selected = false;
    num4 = "";
    ct=true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Count Up/Down:");
    lcd.setCursor(0,1);
    lcd.print("1=Down  2=Up");
  }

  if(key!=NO_KEY && key=='1' || key=='2' && pressed==true && ct==true) {
    if(key=='1') {
      down=true;
    }else if(key=='2') {
      down=false;
    }
  }

  if(key!=NO_KEY && key=='0' || key=='1' || key=='2' || key=='3' || key=='4' || key=='5' || key=='6' || key=='7' || key=='8' || key=='9' && pressed==true) {
    updated = false;
    if(hscore==true) {
      numLength = num1.length();
      if(numLength<3) {
        num1 = num1 + key;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set Home Score:");
        lcd.setCursor(0,1);
        lcd.print(num1);
      }
    }
    if(gscore==true) {
      numLength = num2.length();
      if(numLength<3) {
        num2 = num2 + key;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set Guest Score:");
        lcd.setCursor(0,1);
        lcd.print(num2);
      }
    }
    if(periods==true) {
      num3 = key;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set Period:");
      lcd.setCursor(0,1);
      lcd.print(num3);
    }
    if(timer==true) {
      num7=num7+key;
      numLength = num7.length();
      if(numLength==1) {
        s=key;
        lcd.setCursor(6,1);
        lcd.print(s);

      }else if(numLength==2) {
        S2=s;
        s=key;
        lcd.setCursor(4,1);
        lcd.print(S2);
        lcd.print(".");
        lcd.print(s);

      }else if(numLength==3) {
        S1=S2;
        S2=s;
        s=key;
        lcd.setCursor(3,1);
        lcd.print(S1);
        lcd.print(S2);
        lcd.print(".");
        lcd.print(s);

      }else if(numLength==4) {
        M2=S1;
        S1=S2;
        S2=s;
        s=key;
        lcd.setCursor(1,1);
        lcd.print(M2);
        lcd.print(":");
        lcd.print(S1);
        lcd.print(S2);
        lcd.print(".");
        lcd.print(s);

      }else if(numLength==5) {
        M1=M2;
        M2=S1;
        S1=S2;
        S2=s;
        s=key;
        lcd.setCursor(0,1);
        lcd.print(M1);
        lcd.print(M2);
        lcd.print(":");
        lcd.print(S1);
        lcd.print(S2);
        lcd.print(".");
        lcd.print(s);
      }
    }
  }
  
  if(key!=NO_KEY && key=='y' && pressed==true) {
    if(S1=="6" || S1=="7" || S1=="8" || S1=="9") {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Invalid time");
      M1="1";
      M2="5";
      S1="0";
      S2="0";
      s="0";

      sendData("time");
      digitalWrite(LED_BUILTIN, HIGH);
      
      pressed=false;
      periods=false;
      hscore=false;
      gscore=false;
      timer=false;
      updated=true;
      ct=false;
      used=false;
      hhorn = true;
      lcd.clear();
      homeScreen();

      hhorn = true;
      
    }else{
      used=false;
      if(periods==true) {
        period = num3.toInt();
      }else if(hscore==true) {
        homeScore = num1.toInt();
      }else if(gscore==true) {
        guestScore = num2.toInt();
      }
      pressed=false;
      periods=false;
      hscore=false;
      gscore=false;
      timer=false;
      ct=false;
      lcd.clear();
      homeScreen();
      updated=true;

      hhorn = true;
    }

    sendData("time");
    sendData("hScore");
    sendData("gScore");
    sendData("period");
    sendData("other");

    digitalWrite(LED_BUILTIN, HIGH);
  }
  
  if(key!=NO_KEY && key=='n'&& pressed==true) {

    sendData("time");

    digitalWrite(LED_BUILTIN, HIGH);
    
    selected = false;
    updated = false;
    pressed=false;
    periods=false;
    hscore=false;
    gscore=false;
    lcd.clear();
    homeScreen();
    updated=true;
  }
  
//SCORE
  if(key == 'a' && pressed==false) {
    updated = false;
    homeScore += 1;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Home Score +1");
    lcd.setCursor(0,1);
    lcd.print(homeScore);

    sendData("hScore");

    digitalWrite(LED_BUILTIN, HIGH);
      
  }else if (key == 'b' && pressed==false) {
    updated = false;
    homeScore += 2;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Home Score +2");
    lcd.setCursor(0,1);
    lcd.print(homeScore);

    sendData("hScore");

    digitalWrite(LED_BUILTIN, HIGH);
      
  }else if (key == 'c' && pressed==false) {
    updated = false;
    homeScore += 3;
    lcd.clear();
    lcd.setCursor(0,0);
      lcd.print("Home Score +3");
      lcd.setCursor(0,1);
      lcd.print(homeScore);

      sendData("hScore");

      digitalWrite(LED_BUILTIN, HIGH);
      
  }else if (key == 'd' && pressed==false && homeScore!=0) {
    updated = false;
    homeScore -= 1;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Home Score -1");
    lcd.setCursor(0,1);
    lcd.print(homeScore);

    sendData("hScore");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if (key == 'e' && pressed==false) {
    updated = false;
    guestScore += 1;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Guest Score +1");
    lcd.setCursor(0,1);
    lcd.print(guestScore);

    sendData("gScore");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if (key == 'f' && pressed==false) {
    updated = false;
    guestScore += 2;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Guest Score +2");
    lcd.setCursor(0,1);
    lcd.print(guestScore);

    sendData("gScore");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if (key == 'g' && pressed==false) {
    updated = false;
    guestScore += 3;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Guest Score +3");
    lcd.setCursor(0,1);
    lcd.print(guestScore);

    sendData("gScore");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if (key == 'h' && pressed==false && guestScore!=0) {
    updated = false;
    guestScore -= 1;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Guest Score -1");
    lcd.setCursor(0,1);
    lcd.print(guestScore);

    sendData("gScore");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }

//POSS
  if(key == 'i' && pressed==false) {
    updated = false;
    if(homePoss==false) {
      homePoss = true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Home Possesion");
      lcd.setCursor(0,1);
      lcd.print("On");
    }else{
      updated = false;
      homePoss = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Home Possesion");
      lcd.setCursor(0,1);
      lcd.print("Off");
    }
    
    if(guestPoss == true) {
      guestPoss=false;
    }

    sendData("other");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if (key == 'k' && pressed==false) {
    updated = false;
    if(guestPoss==false) {
      updated = false;
      guestPoss = true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Guest Possesion");
      lcd.setCursor(0,1);
      lcd.print("On");
    }else{
      updated = false;
      guestPoss = false;
      lcd.setCursor(0,0);
      lcd.print("Guest Possesion");
      lcd.setCursor(0,1);
      lcd.print("Off");
    }
    
    if(homePoss == true) {
      homePoss=false;
    }

    sendData("other");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }

//BONUS
  if(key == 'j' && pressed==false) {
    updated = false;
    if(homeBonus==false) {
      homeBonus=true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Home Bonus");
      lcd.setCursor(0,1);
      lcd.print("On");
    }else{
      homeBonus=false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Home Bonus");
      lcd.setCursor(0,1);
      lcd.print("Off");
    }

    sendData("other");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }else if (key == 'l' && pressed==false) {
    updated = false;
    if(guestBonus==false) {
      guestBonus=true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Guest Bonus");
      lcd.setCursor(0,1);
      lcd.print("On");
    }else{
      guestBonus=false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Guest Bonus");
      lcd.setCursor(0,1);
      lcd.print("Off");
    }

    sendData("other");

    digitalWrite(LED_BUILTIN, HIGH);
    
  }

//PERIOD
  if(key=='p' && pressed==false) {
    updated = false;
    period += 1;
    if(period<=9) {
    }else{
      period = 0;
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Period +1");
    lcd.setCursor(0,1);
    lcd.print(period);

    sendData("period");

    digitalWrite(LED_BUILTIN, HIGH);
  }

//HORN
  if(hornState == HIGH) {
    horn==true;
    //lcd.setCursor(7, 0);
    //lcd.print("*");
    sendData("horn");

    digitalWrite(LED_BUILTIN, HIGH);
  }else{
    horn==false;
    //lcd.setCursor(7, 0);
    //lcd.print(" ");
  }
}

void sendData(String item) {
  if(item == "time") {
    String all = "t" + M1 + M2 + ":" + S1 + S2 + "." + s;
    char clk[32];
    all.toCharArray(clk, 32);
    radio.write(&clk, sizeof(clk));
  
  }else if(item == "hScore") {
    String hTot;
    
    if(homeScore < 10) {
      hTot = "h00" + String(homeScore);
    }else if(homeScore < 100) {
      hTot = "h0" + String(homeScore);
    }else{
      hTot = "h" + String(homeScore);
    }
    
    char hSc[32];
    hTot.toCharArray(hSc, 32);
    radio.write(&hSc, sizeof(hSc));
    
  }else if(item == "gScore") {
    String gTot;
    
    if(guestScore < 10) {
      gTot = "g00" + String(guestScore);
    }else if(guestScore < 100) {
      gTot = "g0" + String(guestScore);
    }else{
      gTot = "g" + String(guestScore);
    }
    
    char gSc[32];
    gTot.toCharArray(gSc, 32);
    radio.write(&gSc, sizeof(gSc));
    
  }else if(item == "period") {
    String pTot = "p" + String(period);
    
    char pSc[32];
    pTot.toCharArray(pSc, 32);
    radio.write(&pSc, sizeof(pSc));
    
  }else if(item == "other") {
    String tot;
    
    if(homeBonus == true) {
      tot = "a" + String(homeBonus);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    } if(guestBonus == true) {
      tot = "b" + String(guestBonus);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    } if(homePoss == true) {
      tot = "c" + String(homePoss);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    } if(guestPoss == true) {
      tot = "d" + String(guestPoss);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    }

    if(homeBonus == false) {
      tot = "a" + String(homeBonus);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    } if(guestBonus == false) {
      tot = "b" + String(guestBonus);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    } if(homePoss == false) {
      tot = "c" + String(homePoss);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    } if(guestPoss == false) {
      tot = "d" + String(guestPoss);

      char sc[32];
      tot.toCharArray(sc, 32);
      radio.write(&sc, sizeof(sc));
      
    }
    
  }else if(item == "horn") {
    char hrn[32] = "o";
    radio.write(&hrn, sizeof(hrn));
  }

  digitalWrite(LED_BUILTIN, LOW);
}
