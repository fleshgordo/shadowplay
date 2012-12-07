#include <Servo.h> 
#include <SPI.h>
#include <WiFi.h>
#include <math.h>

Servo Servo1_pan;  // create servo object to control a servo 
Servo Servo1_tilt;

Servo Servo2_pan;  // create servo object to control a servo 
Servo Servo2_tilt;

String currentLine = ""; 
String tweet = ""; 

// PINS
const int mosfet12 =  30;     
const int Light1PWM = 8;
const int Light2PWM = 6;
const int RedPWM = 5;
const int GreenPWM = 2;
const int BluePWM = 3;


// MAXIMUM ratings
const int ServoPanLeftmax = 25;
const int ServoPanRightmax = 160;
const int ServoTiltUpmax = 165;
const int ServoTiltDownmax = 98;

const int ServoTiltCenter = (ServoTiltUpmax + ServoTiltDownmax) /2;
const int ServoPanCenter = (ServoPanLeftmax + ServoPanRightmax) /2;

float radius = 68;

float step1 = 0;
float stepc1 = 0;
float step2 = PI;
float stepc2 = 0;
float rgbStep = 0.0;
float rgbSpeed = 0.0;
float rStep=0.0;
float gStep=0.0;
float bStep=0.0;

float sinus1=0;
float sinus2=PI/4;

float speed1=0.00123f;
float speedc1=0.00304f;
float speed2=0.00123f;
float speedc2=0.000304f;

int oldX1=0;
int oldX2=0;
int oldY1=0;
int oldY2=0;
boolean firstPosition=true;
int sequence=-2;
int sTimer=0;
float lin1=0;
float lin2=0;
float dlin1=0.01;
float dlin2=0.01;

int rrr=0;
int ggg=0;
int bbb=0;

int MAXMAX_BRIGHT = 255;
int MINMAX_BRIGHT = 0;
int MAX_BRIGHT = MAXMAX_BRIGHT;

boolean hasChecked=false;
int darkTimer=0;
int darkTimer2=0;
int lastBright1=0;
int lastBright2=0;

int maxservo = 165;
int minservo = 15;
int red, green, blue;

int Servo1Temp = 2;
int Servo2Temp = 1;
int Servo3Temp = 0;

int pausi=0;

int LightBrightness = 0;

int server_timeout = 0;

float step = 0;
float steps[] = {0, 0.13962634, 0.27925268, 0.41887902, 0.55850536, 0.6981317, 0.83775804,
0.97738438, 1.11701072, 1.25663706, 1.3962634, 1.53588974, 1.67551608, 1.81514242,
1.95476876, 2.0943951, 2.23402144, 2.37364778, 2.51327412, 2.65290046, 2.7925268,
2.93215314, 3.07177948, 3.21140582, 3.35103216, 3.4906585, 3.63028484, 3.76991118,
3.90953752, 4.04916386, 4.1887902, 4.32841654, 4.46804289, 4.60766923, 4.74729557,
4.88692191, 5.02654825, 5.16617459, 5.30580093, 5.44542727, 5.58505361, 5.72467995,
5.86430629, 6.00393263, 6.14355897};

const int steplength = sizeof(steps)/2;


// RGB
float RGB1[3];
float RGB2[3];
float INC[3];

// WIFI STUFF
char ssid[] = "ff20d"; 
char pass[] = "#?1)CNq(Vqvw&y3(lg9";   
char nameserver[] = "10.10.0.31";
int keyIndex = 0;         

boolean wifi_shield = true;
boolean wifi_enable = true;
boolean isConnected=false;

int status = WL_IDLE_STATUS;
IPAddress server(10,10,0,31);
WiFiClient client;

void setup() {
  currentLine.reserve(256);
  tweet.reserve(100);
  Serial.begin(19200);
  // set the digital pin as output:
  pinMode(mosfet12, OUTPUT);
  pinMode(Light1PWM, OUTPUT);
  pinMode(Light2PWM, OUTPUT);
  pinMode(BluePWM, OUTPUT);
  pinMode(RedPWM, OUTPUT);
  pinMode(GreenPWM, OUTPUT);
  set_brightness(5);
  
  digitalWrite(mosfet12, HIGH);
  
  Servo1_pan.attach(22);
  Servo1_tilt.attach(24);
  Servo2_pan.attach(26);
  Servo2_tilt.attach(28);
  
  Servo1_pan.write(90);
  Servo1_tilt.write(110);  
  Servo2_pan.write(90);
  Servo2_tilt.write(110);

  // some rgb randomseeds
  randomSeed(analogRead(6));
  for (int x=0; x<3; x++) {
  RGB1[x] = random(256);
  RGB2[x] = random(256); }

  if (wifi_enable == true) {
   // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present"); 
      wifi_shield = false;
    }    
    if (wifi_shield == true) {
      // attempt to connect to Wifi network:
      while ( status != WL_CONNECTED) { 
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
        status = WiFi.begin(ssid, pass);
        // wait 10 seconds for connection:
        delay(8000);
      } 
      Serial.println("Connected to wifi");
      printWifiStatus();
      
      Serial.println("\nStarting connection to server...");
      // if you get a connection, report back via serial:
      if (client.connect(nameserver, 80)){
        isConnected=true;
        sequence=-1;
        Serial.println("connected to server");
        // Make a HTTP request:
        client.println("GET / HTTP/1.1");
        client.println("Host:localhost");
        client.println("Connection: close");
        client.println();
      }
    }
  }
}

void make_request() {
  Serial.println("\nStarting connection to server...");
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    client.println("GET / HTTP/1.1");
    client.println("Host:localhost");
    client.println("Connection: close");
    client.println();
    server_timeout=0;
    isConnected=true;
  }else{
    isConnected=false;
    server_timeout++;
    if (server_timeout>5){
      sequence=0;
      server_timeout=0;
    }
    Serial.println("server timeout");  }
}

int getIntFromString(String s){
  char ca[s.length()+1];
  s.toCharArray(ca,(s.length()+1));
  return atoi(ca);
}

void server_check() {
  make_request();
  while (client.available()) {
  char inChar = client.read();
  // add incoming byte to end of line:
      //Serial.write(inChar);
      currentLine += inChar;

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      } 
      if ( currentLine.endsWith("</html>")) {
        String content = currentLine.substring(currentLine.indexOf("<body>")+6,currentLine.indexOf("</body>"));
        int kommaIndex1 = content.indexOf(",");
        int kommaIndex2 = content.indexOf(",",kommaIndex1+1);
        int kommaIndex3 = content.indexOf(",",kommaIndex2+1);
        int kommaIndex4 = content.indexOf(",",kommaIndex3+1);
        if(kommaIndex1!=-1){
          String s = content.substring(0,kommaIndex1);
          String br = content.substring(kommaIndex1+1,kommaIndex2);
          String rr = content.substring(kommaIndex2+1,kommaIndex3);
          String gg = content.substring(kommaIndex3+1,kommaIndex4);
          String bb = content.substring(kommaIndex4+1);
          sequence=getIntFromString(s);
          MAX_BRIGHT=getIntFromString(br);
          rrr=getIntFromString(rr);
          ggg=getIntFromString(gg);
          bbb=getIntFromString(bb);
          Serial.println();
          Serial.print("sequence: ");
          Serial.println(sequence);
          Serial.print("brightness: ");
          Serial.println(MAX_BRIGHT);
          Serial.print("red: ");
          Serial.println(rr);
          Serial.print("green: ");
          Serial.println(gg);
          Serial.print("blu: ");
          Serial.println(bb);
        }
      }
      
  }

}
void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void set_brightness(int brightness) {
    int new_brightness = map(brightness, 0, 100, 255, 0);
    analogWrite(BluePWM,new_brightness);
    analogWrite(RedPWM,new_brightness);
    analogWrite(GreenPWM,new_brightness);
    analogWrite(Light1PWM,new_brightness);
    analogWrite(Light2PWM,new_brightness);
  return;
}

void rgbled(int r1,int g1, int b1){
    b1=b1/2;
    r1=(r1*2)/3;
    int r2 = map(r1, 0, 255, MAXMAX_BRIGHT, MAXMAX_BRIGHT-MAX_BRIGHT);
    int g2 = map(g1, 0, 255, MAXMAX_BRIGHT, MAXMAX_BRIGHT-MAX_BRIGHT);
    int b2 = map(b1, 0, 255, MAXMAX_BRIGHT, MAXMAX_BRIGHT-MAX_BRIGHT);
        
    analogWrite(GreenPWM,g2);
    analogWrite(BluePWM,b2);
    analogWrite(RedPWM,r2);
}


void bright1(int brightness){
  if(brightness<15)brightness=15;
  lastBright1=brightness;
  int new_brightness = map(brightness, 0, 255, MAXMAX_BRIGHT, MAXMAX_BRIGHT-MAX_BRIGHT);
  analogWrite(Light1PWM,new_brightness);
}
void bright2(int brightness){
  if(brightness<15)brightness=15;
  lastBright2=brightness;
  int new_brightness = map(brightness, 0, 255, MAXMAX_BRIGHT, MAXMAX_BRIGHT-MAX_BRIGHT);
  analogWrite(Light2PWM,new_brightness);
}

void writePan(Servo servo, int svalue){
  if(svalue < ServoPanLeftmax)svalue=ServoPanLeftmax;
  if(svalue > ServoPanRightmax)svalue=ServoPanRightmax;
  servo.write(svalue);
}
void writeTilt(Servo servo, int svalue){
  if(svalue < ServoTiltDownmax)svalue=ServoTiltDownmax;
  if(svalue > ServoTiltUpmax)svalue=ServoTiltUpmax;
  servo.write(svalue);
}

void draw_circle(Servo servo_pan, Servo servo_tilt, int r, int i) 
{
  int x = float(ServoPanCenter) + float(r) * cos(i);
  int y = float(ServoTiltCenter) - float(r) * sin(i);
  writePan(servo_pan,x);
  writeTilt(servo_tilt,y);
     
//   if (x > ServoPanLeftmax && x < ServoPanRightmax && y < ServoTiltUpmax && y > ServoTiltDownmax) {
//    servo_pan.write(x);
//    servo_tilt.write(y);
//    Serial.print("x: ");
//   Serial.print(x);
//    Serial.print("y: ");
//   Serial.print(y);
//   Serial.println();
//   delay(delaytime);
//   }
// }
 return;
}

// move the servo a given amount
void moveServo(Servo servo, int delta) {
 int previousValue = servo.read();
 int newValue = previousValue + delta;
 if (newValue > maxservo || newValue < minservo) {
  return;
 }
 servo.write(newValue);
 Serial.println(newValue);
}

void color (unsigned int red, unsigned int green, unsigned int blue) { 	 
  analogWrite(RedPWM, 255-red); 	 
  analogWrite(BluePWM, 255-blue);
  analogWrite(GreenPWM, 255-green);
  
  return;
}


void rgbBlink(float blink1,float blink2){
    rgbStep+=blink1;
    rgbSpeed+=blink2;
    int rr=float(rrr)* (sin(rgbSpeed)+1)/2;
    int gg=float(ggg)* (sin(rgbSpeed)+1)/2;
    int bb=float(bbb)* (sin(rgbSpeed)+1)/2;
    rgbled(rr,gg,bb);
}
void rgbColorBlink(float blink1,float blink2){
    rgbStep+=blink1;
    rgbSpeed+=blink2;
    int rr = 127+127.0f*sin(rgbStep);
    int gg = 127+127.0f*cos(rgbStep);
    int bb = 127+127.0f*sin(rgbStep+(PI/2));
    rr=float(rr)* (sin(rgbSpeed)+1)/2;
    gg=float(gg)* (sin(rgbSpeed)+1)/2;
    bb=float(bb)* (sin(rgbSpeed)+1)/2;
    rgbled(rr,gg,bb);
}

void gBlink(float blink1){
  gStep+=blink1;
  int gg = 127+127.0f*cos(gStep);
  rgbled(0,gg,0);
}
void rBlink(float blink1){
  gStep+=blink1;
  int rr = 127+127.0f*cos(gStep);
  rgbled(rr,0,0);
}


void movement0(){
    step1+=speed1;
    step2+=speed2;
    stepc1+=speedc1;
    stepc2+=speedc2;
    int x1 = float(ServoPanCenter) + (radius*0.8) * cos(step1);
    int y1 = float(ServoTiltCenter-20) - (radius*0.14)  * sin(stepc1);
    int x2 = float(ServoPanCenter) + (radius*0.76) * cos(-step2);
    int y2 = float(ServoTiltCenter-20) - (radius*0.16) * sin(-stepc2);
    writePan(Servo1_pan,x1);
    writePan(Servo2_pan,x2);
    writeTilt(Servo1_tilt,y1);
    writeTilt(Servo2_tilt,y2);
}

void movement1(){
    step1+=speed1/2.25;
    step2+=speed2/2.25;
    
    float cosStep1=cos(step1);
    float cosStep2=cos(step2);
    float sinStep1=sin(step1);
    float sinStep2=sin(step2);
    float cosQ1=cosStep1*cosStep1;
    float cosQ2=cosStep2*cosStep2;
    
    int x1 = float(ServoPanCenter) + radius * cosStep1;
    int y1 = float(ServoTiltCenter-15) - (radius*0.02)  * sinStep1;
    int x2 = float(ServoPanCenter) + radius * cosStep2;
    int y2 = float(ServoTiltCenter-25) - (radius*0.02) * sinStep2;
  
    writePan(Servo1_pan,x1);
    writeTilt(Servo1_tilt,y1);
    writePan(Servo2_pan,x2);
    writeTilt(Servo2_tilt,y2);
    
    //movement1 is coupled with light
    lastBright1 = 255 - 255.0 * cosQ1;
    if(sinStep1>=0)lastBright1=0;
    bright1(lastBright1);
    lastBright2 = 255 - 255.0 * cosQ2;
    if(sinStep2>=0)lastBright2=0;
    bright2(lastBright2);
}

void randomServo(){
      int x1 = random(ServoPanLeftmax,ServoPanRightmax);
      int x2 = random(ServoPanLeftmax,ServoPanRightmax);
      int y1 = random(ServoTiltDownmax,ServoTiltUpmax);
      int y2 = random(ServoTiltDownmax,ServoTiltUpmax);
      writePan(Servo1_pan,x1);
      writePan(Servo2_pan,x2);
      writeTilt(Servo1_tilt,y1);
      writeTilt(Servo2_tilt,y2);
  
}

void checkDarkness(){
  if(lastBright1+lastBright2<16){
    if(!hasChecked){
      //darkTimer=0;
      darkTimer2=0;
      server_check();
      hasChecked=true;
    }else{
      darkTimer2++;
      int limit=10000;
      if(sequence==0)limit=15500;
      if(darkTimer2>limit){
        darkTimer2=0;
        hasChecked=false;
      }
    }
  }else{
    darkTimer++;
    hasChecked=false;
    if(darkTimer>15000){
      darkTimer=0;
      server_check();
    }
  }
}

void loop()
{

  
  //set_brightness(LightBrightness);
  //if(isConnected){
    if(sequence>=0){
      checkDarkness();
    }
  //}
    
  if(sequence==-2){
    digitalWrite(mosfet12,HIGH);
    rBlink(0.0043);
    sTimer++;
    if(sTimer>8000){
      sTimer=0;
      //sequence=1;
      MAX_BRIGHT=128;
      server_check();
    }
  }else if(sequence==-1){
    digitalWrite(mosfet12,HIGH);
    gBlink(0.0043);
    sTimer++;
    if(sTimer>8000){
      sTimer=0;
      sequence=1;
      server_check();
    }
  }else if(sequence==0){
    // reset servo position
//    Servo1_pan.write(90);
//    Servo1_tilt.write(120);  
//    Servo2_pan.write(90);
//    Servo2_tilt.write(120);
//    delay(250);
    digitalWrite(mosfet12,LOW);
    rgbled(0,0,0);
    
  }else if(sequence==4){
    digitalWrite(mosfet12,HIGH);
    //sequence=-1;
    movement0();
    sinus1+=0.0012;
    sinus2-=0.0023;
    bright1(200*(sin(sinus1)+1)/2);
    bright2(200*(sin(sinus2)+1)/2);
    rgbBlink(0.0042,0.0017);
  }else if(sequence==2){
    digitalWrite(mosfet12,HIGH);
    rgbBlink(0.00093,0.00077);
    movement1();
  }else if(sequence==1){
    digitalWrite(mosfet12,HIGH);
    if(pausi==0){
      lin1+=dlin1;
      if(lin1>1)dlin1=-0.0001;
      if(lin1<-0.05)dlin1=0.00010;
      lin2+=dlin2;
      if(lin2>1)dlin2=-0.0001;
      if(lin2<-0.05){
        dlin2=0.0001;
        pausi++;
      }
    }else{
        pausi++;
        if(pausi>1000){
          randomServo();
          pausi=0;
        }
    }
    
    float hlin1=lin1;
    float hlin2=lin2;
    if(hlin1<0){
      hlin1=0;
    }
    if(hlin2<0){
      hlin2=0;
    }
    lastBright1 = 255.0 * hlin1;
    lastBright2 = 255.0 * hlin2;
    bright1(lastBright1);
    
    int lastBright3 = 255-lastBright1;
    bright2(lastBright1);
    
    lastBright1=lastBright1/2;
    
    //rgbled(lastBright1,lastBright1,lastBright1);
    rgbled(0,0,0);
    //rgbBlink(0.0142,0.0057);
    
    
    
  }else if(sequence==3){
    digitalWrite(mosfet12,HIGH);
    lin1+=dlin1;
    if(lin1>1)dlin1=-0.0001;
    if(lin1<0)dlin1=0.0001;
    lin2+=dlin2;
    if(lin2>1)dlin2=-0.0001;
    if(lin2<0)dlin2=0.0001;
    
    lastBright1 = 255 - 255.0 * lin1;
    lastBright2 = 255 - 255.0 * lin2;
    bright1(lastBright1);
    
    int lastBright3 = 255-lastBright1;
    bright2(lastBright1);
    
    lastBright1=lastBright1/2;
    
    rgbBlink(0.0142,0.0057);
    
    //rgbled(lastBright1,lastBright1,lastBright1);
    movement0();
  }
  
  
  
  if (Serial.available() >0) {
    byte incoming = Serial.read();
    // turn off 12V power for shields
    if (incoming == 'q') {
      digitalWrite(mosfet12, LOW);
      Serial.println("mosfet off");
    }
    // turn on
    else if (incoming == 'o') {
      digitalWrite(mosfet12, HIGH);
    }
    
    else if(incoming == '0'){
      sequence=0;
    }else if(incoming == '1'){
      sequence=1;
    }else if(incoming == '2'){
      sequence=2;
    }else if(incoming == '3'){
      sequence=3;
    }else if(incoming == '4'){
      sequence=4;
    }

    //set speed
    else if (incoming == '5'){
      speed1-=0.00013;
      speed2-=0.00013;
    }
    else if (incoming == '6'){
      speed1+=0.00013;
      speed2+=0.00013;
    }else if(incoming == '7'){
      MAX_BRIGHT-=5;
      if(MAX_BRIGHT<MINMAX_BRIGHT)MAX_BRIGHT=MINMAX_BRIGHT;
    }else if(incoming == '8'){
      MAX_BRIGHT+=5;
      if(MAX_BRIGHT>MAXMAX_BRIGHT)MAX_BRIGHT=MAXMAX_BRIGHT;
    }
    // motor circles
    else if (incoming == 'm') {  
      for (int j=0;j<100;j+=20) {
         set_brightness(j);
         //draw_circle(Servo1_pan, Servo1_tilt, 50);
         //wdraw_circle(Servo2_pan, Servo2_tilt, 50);
      }
      
    }
    // motors right 1step
    else if (incoming == 's') {  
       moveServo(Servo2_pan,1);
       moveServo(Servo1_pan,1);
    }
    // motors left 1step
    else if (incoming == 'a') {  
       moveServo(Servo2_pan,-1);
       moveServo(Servo1_pan,-1);
    }   
    // motors up 1step
    else if (incoming == 'w') {  
       moveServo(Servo2_tilt,1);
       moveServo(Servo1_tilt,1);
    }
    // motors down 1step
    else if (incoming == 'z') {  
       moveServo(Servo2_tilt,-1);
       moveServo(Servo1_tilt,-1);
    }    
    // increase brightness
    else if (incoming == 'd') {
       LightBrightness +=1;
    }
    // decrease brightness
    else if (incoming == 'c') {
      LightBrightness -=1;
    }
    // increase brightness faster
    else if (incoming == 'f') {
      LightBrightness +=10;
    }
    // decrease brightness faster
    else if (incoming == 'v') {
      LightBrightness -=10;
    }
    // set full brightness
    else if (incoming == 'g') {
      set_brightness(100);
      LightBrightness = 100;
    }
    // check webclient
    else if (incoming == 'k') {
      server_check();
    }
    // test rgb aka double rainbow
    else if (incoming == 'u') {
       for (int i=0;i<255;i++) {
        analogWrite(RedPWM,255-i);
        analogWrite(BluePWM,255);
        analogWrite(GreenPWM,255);
        delay(5);
      }    
       for (int i=0;i<255;i++) {
        analogWrite(RedPWM,i);
        analogWrite(BluePWM,255);
        analogWrite(GreenPWM,255);
        delay(5);
      }     
      for (int i=0;i<255;i++) {
        analogWrite(GreenPWM,255-i);
        analogWrite(BluePWM,255);
        analogWrite(RedPWM,255);
        delay(5);
      }
      for (int i=0;i<255;i++) {
        analogWrite(GreenPWM,i);
        analogWrite(BluePWM,255);
        analogWrite(RedPWM,255);
        delay(5);
      }
      for (int i=0;i<255;i++) {
        analogWrite(BluePWM,255-i);
        analogWrite(RedPWM,255);
        analogWrite(GreenPWM,255);
        delay(5);
      }
       for (int i=0;i<255;i++) {
        analogWrite(BluePWM,i);
        analogWrite(RedPWM,255);
        analogWrite(GreenPWM,255);
        delay(5);
      }

    }
    else if (incoming == 'y') {
       
    }
    else if (incoming == 't') {
      int Servo1CurrentTemp = analogRead(Servo1Temp);
      Serial.print("Servo1 Temp: ");
      Serial.println(Servo1CurrentTemp);
      int Servo2CurrentTemp = analogRead(Servo2Temp);
      Serial.print("Servo2 Temp: ");
      Serial.println(Servo2CurrentTemp);
      int Servo3CurrentTemp = analogRead(Servo3Temp);
      Serial.print("Servo3 Temp: ");
      Serial.println(Servo3CurrentTemp);
      
    }
  }
}

