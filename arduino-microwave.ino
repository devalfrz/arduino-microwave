/*
  Microwave: Control a microwave using an Arduino

  Alfredo Rius
  alfredo.rius@gmail.com

  v2.0   2017-12-11
  - Code Rewrite.
  - Using timer interrupts.

  v1.1   2017-12-02
  - Cleaned minimum time display.
  - Changed range of times.
  - Added button debounce.

  v1.0   2017-09-17
  - Recycled code from other project.
  - Developed in a hurry, but it works.

*/

#define BUTTON 3
#define LIGHT A5
#define GEN A4
#define POT A0
#define CHANGE_THR 2
#define DEBOUNCE_FILTER 500

#include <LiquidCrystal.h>
#include <TimerOne.h>

LiquidCrystal lcd(4, 5, 7, 8, 12, 13);

#define ERR         0
#define STAND_BY    1
#define COUNTDOWN   2
#define SHUTDOWN    3
#define CANCEL      4
uint8_t state = STAND_BY;

unsigned long debounceFilter = 0;

unsigned long time_left;


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
  lcd.print("Microwave   v2.0");
  lcd.setCursor(0,1);
  lcd.print("                ");
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("Set Time:       ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  Timer1.initialize(1000000);
  set_state(STAND_BY);
}

void loop() {
  unsigned long t = get_time(analogRead(POT));
  if((t<time_left-CHANGE_THR || t>time_left+CHANGE_THR) && state == STAND_BY){
    time_left = t;
    display_time();
  }
}

void display_time(){
  // Display the time
  float tmp;
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  tmp = time_left/60;
  if(tmp<10)
    lcd.print("0");
  lcd.print((int)tmp);
  lcd.print(":");
  tmp = time_left - ((time_left/60)*60);
  if(tmp<10)
    lcd.print("0");
  lcd.print((int)tmp);
}

uint8_t set_state(uint8_t new_state){
  // Sets the microwave to a specific state.
  if(new_state == STAND_BY){
    digitalWrite(LIGHT,LOW);
    digitalWrite(GEN,LOW);
    Timer1.detachInterrupt();
    lcd.setCursor(0, 0);
    lcd.print("Set Time:       ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }else if(new_state == SHUTDOWN || new_state == CANCEL){
    lcd.setCursor(0,0);
    if(new_state == SHUTDOWN)
      lcd.print("     DONE!!!    ");
    else
      lcd.print("    STOPPED!    ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    time_left = 3;
    digitalWrite(LIGHT,HIGH);
    digitalWrite(GEN,LOW);
  }else if(new_state == COUNTDOWN){
    lcd.setCursor(0, 0);
    lcd.print("Time Left:      ");
    time_left = get_time(analogRead(POT));
    digitalWrite(LIGHT,HIGH);
    digitalWrite(GEN,HIGH);
    Timer1.attachInterrupt(countdown);
  }else{
    return ERR;
  }
  state = new_state;
  return new_state;
}


long get_time(unsigned int pot){
  // Calculate the time from the raw input.
  if(pot<100){
    return 30;
  }else if(pot<800){
    return map(pot,100,800,2,20)*30;
  }else{
    return map(pot,801,1024,10,31)*60;
  }
}


void set(){
  // Push button interrupt
  if(debounceFilter<millis()){
    if(state == STAND_BY){
      set_state(COUNTDOWN);
    }else if(state == COUNTDOWN){
      set_state(CANCEL);
    }
    debounceFilter = millis()+DEBOUNCE_FILTER;
  }
}

void countdown(){
  // Calculate countdown
  if(state == COUNTDOWN){
    if(time_left > 0){
      time_left--;
      display_time();
    }else{
      set_state(SHUTDOWN);
    }
  }else if(state == SHUTDOWN || state == CANCEL){
    if(time_left > 0)
      time_left--;
    else
      set_state(STAND_BY);
  }
}

