/*
  Microwave: Control a microwave usin an Arduino

  Alfredo Rius
  alfredo.rius@gmail.com


  v1.0   2017-09-17
  - Recycled code from other project.
  - Developed in a hurry, but it works.

  v1.1   2017-12-02
  - Cleaned minimum time display.
  - Changed range of times.
  - Added button debounce.

*/

#define BUTTON 3
#define LIGHT A5
#define GEN A4
#define POT A0
#define CHANGE_THR 2
#define DEBOUNCE_FILTER 500

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(4, 5, 7, 8, 12, 13);

byte state = 0;
// 0: Stand by
// 1: Countdown
// 2: Shutting down

unsigned int raw_time = 0;
long seconds = 0;

unsigned long debounceFilter = 0;

unsigned long time_left;
unsigned long time_elapsed;
unsigned long score = 0;
boolean first_run = true;


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(GEN, OUTPUT);
  pinMode(LIGHT, OUTPUT);
  pinMode(POT, INPUT);
  digitalWrite(LIGHT,LOW);
  digitalWrite(GEN,LOW);
  attachInterrupt(digitalPinToInterrupt(BUTTON), set, FALLING);
  lcd.setCursor(0,0);
  lcd.print("Microwave   v1.1");
  lcd.setCursor(0,1);
  lcd.print("                ");
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("Set Time:       ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  
}

void loop() {
  if(state == 1){
     delay(50);
     display_time();
  }else if(state == 0){
    set_time();
    delay(20);
  }else if(state == 2){
    delay(3000);
    lcd.setCursor(0,0);
    lcd.print("Set Time:       ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    state = 0;
    raw_time = 0;
    digitalWrite(LIGHT,LOW);
    digitalWrite(GEN,LOW);
    preview_time();
  }
}

void set_time(){
  unsigned int pot = analogRead(POT);
  if(pot<raw_time-CHANGE_THR || pot>raw_time+CHANGE_THR){
    raw_time = pot;
    preview_time();
  }
}

void preview_time(){
  float tmp;
  if(raw_time<100){
    if(seconds != 30){
      lcd.setCursor(0,1);
      lcd.print("00:30           ");
    }
    seconds = 30;
  }else{
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    tmp = ((raw_time-100)/50.0)+1;
    if(tmp<10)
      lcd.print("0");
    lcd.print((int)tmp);
    seconds = (int)tmp * 60;
    if(tmp-(int)tmp>=0.5){
      lcd.print(":30");
      seconds += 30;
    }else{
      lcd.print(":00");
    }
  }
  //lcd.setCursor(10,1);
  //lcd.print(raw_time);
}

void set(){
  if(debounceFilter<millis()){
    if(state == 0){
      lcd.setCursor(0,0);
      lcd.print("Time Left:       ");
      state = 1;
      reset_timer();
      digitalWrite(LIGHT,HIGH);
      digitalWrite(GEN,HIGH);
    }else{
      state = 2;
      digitalWrite(GEN,LOW);
      lcd.setCursor(0,0);
      lcd.print("    STOPPED!    ");
      lcd.setCursor(0,1);
      lcd.print("                ");
    }
    debounceFilter = millis()+DEBOUNCE_FILTER;
  }
  
}

void display_time(){
  unsigned long int tmp;
  //time_elapsed = millis() - time_left;
  time_elapsed = time_left - millis();
  if(time_elapsed <= 3600000){// Max time is 1 hr
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    tmp = time_elapsed/60000;
    if(tmp<10) lcd.print("0");
    lcd.print(tmp);
    lcd.print(":");
    tmp = (time_elapsed/1000)-(tmp*60);
    if(tmp<10) lcd.print("0");
    lcd.print(tmp);
  }else{
    state = 2;
    digitalWrite(GEN,LOW);
    lcd.setCursor(0,0);
    lcd.print("     DONE!      ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    delay(3000);
    lcd.setCursor(0,0);
    lcd.print("Set Time:       ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    state = 0;
    raw_time = 0;
    digitalWrite(LIGHT,LOW);
    digitalWrite(GEN,LOW);
    preview_time();
  }
}  
void reset_timer(){
  time_left = millis()+(seconds*1000);
  //time_left = millis();
}
