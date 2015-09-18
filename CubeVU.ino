#define  NUM_TLCS  4
#define  NUM_ROWS  8
#include "Tlc5940Mux.h"
void (*vuLib[4]) (unsigned int);

byte z=2;
byte y=1;
byte x=0;

byte vert3[3]={1,1,1};
byte vert4[3]={6,6,6};
unsigned long sTime;

unsigned long vuDur = 10000;
unsigned int maxPwm=1000;
unsigned int noise = 300;
unsigned int top = 1024;
unsigned int samples = 20;

byte input= A4;

ISR(TIMER1_OVF_vect)
{
  static byte counter;
  static byte row;
  
  if (counter==4) {
    counter=0;
    digitalWrite(5,1);  
    TlcMux_shiftRow(row);
    latch();
    turnOn(row);
    row++;
    if(row>7) row=0;
  }
  counter++;
}

void setup()
{
  Serial.begin(9600);
  delay(10);
  //pin stuf
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(4,0);
  digitalWrite(5,0);
  //tlc stuff
  TlcMux_init();
  disable_XLAT_pulses();
  TlcMux_clear();
  //array stuf
  vuLib[0] = solidCubeVU;
  vuLib[1] = towerBotVU;
  vuLib[2] = voidCubeVU;
  vuLib[3] = towerTopVU;
  
  delay(1);
  
  //test staff
  
  //drawSolidCube( vert1 , vert2, maxPwm);
  //solidCubeVU(20000);
  //towerBotVU(20000);
  //voidCubeVU(20000);
  //drawShellCube(vert1, vert2, maxPwm);
  //drawLineCube(vert1, vert2, maxPwm);
}



void loop()
{
  static byte iter;
  unsigned int data = audioInput1();
  (*vuLib[iter])(data);
  if(deltaT(sTime) > vuDur){
    sTime=millis();
    iter++;
    if(iter==4) iter = 0;
  }  
}

void turnOn(byte lvl)
{
  if(lvl>7){
    TlcMux_shift8(~0);
  }
  else {
    TlcMux_shift8(~(1<<lvl));
  }
  digitalWrite(4, 1);
  digitalWrite(4, 0);
  digitalWrite(5, 0);
}

void latch()
{
  XLAT_PORT |= _BV(XLAT_PIN);
  XLAT_PORT &= ~_BV(XLAT_PIN);
  BLANK_PORT |= _BV(BLANK_PIN);
  TCNT1=0;
  BLANK_PORT &= ~_BV(BLANK_PIN);
  
}

void updateCube()
{
  for(byte i =0; i<8; i++){
    turnOn(8);
    TlcMux_shiftRow(i);
    //digitalWrite(5,1);
    latch();
    turnOn(i);
    delay(1);
  }   
}

unsigned int audioInput1()
{
  byte peak=0;
  for(byte i =0; i<samples; i++){
    byte raw = analogRead(input);
    if(raw>peak) peak=raw;
  }
  return map(peak-noise, 0, top, 0, 0xFFFF);
}
  

void solidCubeVU(unsigned int signal)
{
  TlcMux_clear();
  byte quant = map(signal, 0, 65535, 0, 4);
  byte vert1[3]={4, 4, 4};
  byte vert2[3]={3, 3, 3};
  if(quant>0){
    for(byte i=0; i<3; i++){
    vert1[i]-=quant;
    vert2[i]+=quant;
    }
    drawSolidCube(vert1, vert2, maxPwm);
  }
  if(quant<4){
    for(byte i=0; i<3; i++){
      vert1[i]--;
      vert2[i]++;
    }
    drawShellCube(vert1, vert2, map(signal-(16383*quant), 0, 16383, 0, maxPwm));
  }
  
}
void voidCubeVU(unsigned int signal)
{
  TlcMux_clear();
  byte quant = map(signal, 0, 65535, 0, 4);
  byte vert1[3]={4, 4, 4};
  byte vert2[3]={3, 3, 3};
  if(quant>0){
    for(byte i=0; i<3; i++){
    vert1[i]-=quant;
    vert2[i]+=quant;
    }
    drawShellCube(vert1, vert2, maxPwm);
  }
  if(quant<4){
    for(byte i=0; i<3; i++){
      vert1[i]--;
      vert2[i]++;
    }
    drawLineCube(vert1, vert2, map(signal-(16383*quant), 0, 16383, 0, maxPwm));
  }
  
}

void towerTopVU(unsigned int signal)
{
  TlcMux_clear();
  byte quant = map(signal, 0, 65535, 0, 7);
  for(byte lvl = 0; lvl<=quant; lvl++){
    for(byte line = 0; line <8; line++){
      for(byte led = 0; led <8; led++){
        TlcMux_set(lvl,(line*8)+led,maxPwm);
      }
    }
  }
  if(quant<7){
    unsigned int lastPwm = map(signal-(9362*quant), 0, 9362, 0, maxPwm);
    for(byte line = 0; line < 8; line++){
      for(byte led =0; led <8; led++){
        TlcMux_set(quant+1,(line*8)+led,lastPwm);
      }
    }
  }  
}
void towerBotVU(unsigned int signal)
{
  TlcMux_clear();
  byte quant = map(signal, 0, 65535, 0, 7);
  for(byte lvl = 7; lvl>=7-quant; lvl--){
    for(byte line = 0; line <8; line++){
      for(byte led = 0; led <8; led++){
        TlcMux_set(lvl,(line*8)+led,maxPwm);
      }
    }
  }
  if(quant<7){
    unsigned int lastPwm = map(signal-(9362*quant), 0, 9362, 0, maxPwm);
    for(byte line = 0; line < 8; line++){
      for(byte led =0; led <8; led++){
        TlcMux_set(7-(quant+1),(line*8)+led,lastPwm);
      }
    }
  }  
}
  
void drawSolidCube(byte vert1[3], byte vert2[3], unsigned int value)
{
  for(byte lvl = vert1[z]; lvl<=vert2[z]; lvl++){
    for(byte line = vert1[y]; line <=vert2[y]; line++){
      for(byte led = vert1[x]; led <=vert2[x]; led++){
        TlcMux_set(lvl,(line*8)+led,value);
      }
    }
  }
  
}

void drawShellCube(byte vert1[3], byte vert2[3], unsigned int value)
{
  for(byte lvl = vert1[z]; lvl<=vert2[z]; lvl++){
    for(byte line = vert1[y]; line <=vert2[y]; line++){
      for(byte led = vert1[x]; led <=vert2[x]; led++){
        if(led == vert1[x]||led==vert2[x]||line==vert1[y] || line==vert2[y] ||lvl==vert1[z] || lvl==vert2[z]){
          TlcMux_set(lvl,(line*8)+led,value);
        }
      }
    }
  }
}

void drawLineCube(byte vert1[3], byte vert2[3], unsigned int value)
{
  for(byte xPlane=vert1[x]; xPlane<=vert2[x]; xPlane++)
  {
    TlcMux_set(vert1[z],xPlane+(vert1[y]*8),value);
    TlcMux_set(vert2[z],xPlane+(vert1[y]*8),value);
    TlcMux_set(vert1[z],xPlane+(vert2[y]*8),value);
    TlcMux_set(vert2[z],xPlane+(vert2[y]*8),value);
  }
  for(byte yPlane=vert1[y]; yPlane< vert2[y]; yPlane++)
  {
    TlcMux_set(vert1[z],vert1[x]+(yPlane*8),value);
    TlcMux_set(vert1[z],vert2[x]+(yPlane*8),value);
    TlcMux_set(vert2[z],vert1[x]+(yPlane*8),value);
    TlcMux_set(vert2[z],vert2[x]+(yPlane*8),value);
  }
  for(byte zPlane=vert1[z]; zPlane< vert2[z]; zPlane++)
  {
    TlcMux_set(zPlane, vert1[x]+(vert1[y]*8), value);
    TlcMux_set(zPlane, vert2[x]+(vert1[y]*8), value);
    TlcMux_set(zPlane, vert1[x]+(vert2[y]*8), value);
    TlcMux_set(zPlane, vert2[x]+(vert2[y]*8), value);
  }
  
}

void serialEvent()
{
  String input = Serial.readStringUntil(10);
  unsigned int data = input.toInt();
  //solidCubeVU(data);
  voidCubeVU(data);
  //towerTopVU(data);
}

unsigned long deltaT(unsigned long start)
{
  if(start<=millis()){
    return millis()-start;
  }
  else {
    return (0xffffffff-start)+millis();
  }
}
