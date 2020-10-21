
#include <LedControl.h>

LedControl leds = LedControl(4,7,6,1);

/* Pins */
int latchP = 12;
int clockP = 11;
int dataP = 13;
int soundAnalogP = A0;
int soundDigP = 2;
int led13 = 13; //built-in LED
int led1 = 8;
int led2 = 9;
int led3 = 10;

byte leds_8 = 0;
float c1 = 0.999;
float c2 = 0.99;
float stDev = 0.0; //standard deviation
float v_avg = 0.0; //signal mean
float dev_scale, stDev2, no_ledsLit;
int j = 0;

void setup() {
  leds.shutdown(0,false);
  leds.setIntensity(0,4); //medium intensity
  leds.clearDisplay(0);
  Serial.begin(9600);
  pinMode(soundDigP, INPUT);
  pinMode(latchP, OUTPUT);
  pinMode(dataP, OUTPUT);
  pinMode(clockP, OUTPUT);
  pinMode(led13, OUTPUT);
  pinMode(leds_8, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
}

void loop() {
  float analogVal = analogRead(soundAnalogP); 
  soundSensor();
  bool yes = processing(analogVal);
  /*Once the standard deviation of the sound signal is within a small range
  the LEDs and LED matrix changes fairly accurately.*/
  if (yes == true){
    if (j%20 == 0){ //to prevent the Led Matrix from changing to quickly.
     tryM();
    }
  }
  j++;
}

void updShiftReg(){
  digitalWrite(latchP,LOW);
  shiftOut(dataP,clockP,LSBFIRST, leds_8);
  digitalWrite(latchP, HIGH);
}

void soundSensor(){
  int digVal=digitalRead(soundDigP); 
  
  if(digVal==HIGH)      
  {
    digitalWrite(led13,HIGH);
  }
  else
  {
    digitalWrite(led13,LOW);
  }
}

int processing(float v){
  /*https://www.dsprelated.com/freebooks/mdft/Signal_Metrics.html 
  The first section on this webpage will help understand some the equations below.*/
  bool matrix;
  float factor = 0.4;

  v_avg= c1*v_avg + (1.0-c1)*v;
  stDev2 = c2*stDev*stDev + (1.0-c2)*(v-v_avg)*(v-v_avg);
  stDev = sqrt(stDev2);
  Serial.println(stDev);
  no_ledsLit = stDev*factor;
  dev_scale = no_ledsLit/30;

  //This change the three LEDs on the right.
  if (dev_scale<0.1) {digitalWrite(led1, LOW);} else{digitalWrite(led1,HIGH);}
  if (dev_scale<0.2) {digitalWrite(led2, LOW);} else{digitalWrite(led2,HIGH);}
  if(dev_scale<0.3) {digitalWrite(led3, LOW);} else{digitalWrite(led3,HIGH);}

  /*This changes eight LEDs on the left and is changed by a shift register
  so that less arduino pins are used.*/
  if (no_ledsLit >8){ no_ledsLit = 8;
  }else{
    leds_8 = 0; //initially no LEDS are on
    for (int i=0; i<no_ledsLit; i++){
      leds_8 = leds_8 + (1<<i); 
    }
  }
  updShiftReg();
  leds_8 = 0;
  updShiftReg();
  if (stDev < 100){
    matrix = true; 
  }else{
    matrix = false;
  }
  return matrix;
}

void tryM(){
  leds.clearDisplay(0);
  if (no_ledsLit >8){ 
    for(int row=0; row< 8; row++){
      for(int col=0; col<8; col++){
        leds.setLed(0, row, col, true);
      }
    }
  }else{
    for(int row=0; row< 8; row++){
      int cols = (int)no_ledsLit*(row+1)/4.5;
      for(int col=0; col<cols; col++){
        leds.setLed(0, row, col, true);
      }
    }
  }
}
