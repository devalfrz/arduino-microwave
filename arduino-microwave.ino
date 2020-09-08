/**
    Microwave firmware
    @file arduino-microwave.ino
    @author Alfredo Rius
    @email alfredo.rius@gmail.com
    @version 3.0 2020-09-08

    Runs on a Teensy3.2 with a TM1637 7-segment Display Driver
    and an encoder.

    Controls the magnetron contactor and the light and fan relay.
*/

#include <TM1637Display.h>

// Pinouts
#define ENC_01 3   // Encoder CLK
#define ENC_02 4   // Encoder direction
#define ENC_SW 5   // Encoder push button
#define DISP_CLK 6 // Display CLK
#define DISP_DIO 7 // Display data I/O
#define LIGHT 11   // Light and fan output
#define GEN 12     // Magnetron outout

// End countdown (show "dOnE" for X seconds)
#define END_COUNT 3

// Countdown max 15:00
#define COUNT_MAX 1500

// Brightness countdown
#define BRIGHTNESS_COUNT 3000 //ms
#define BRIGHTNESS_HIGH 4
#define BRIGHTNESS_LOW 0

// Debounce filter timeout (ms)
#define DEBOUNCE_FILTER 30

// Count timer
unsigned int count = 130; // Start at 1:30
unsigned int last_count = count; // Restart counter to last_count

// Debounce filters
unsigned long debounce_filter_enc = 0;
unsigned long debounce_filter_sw = 0;

// Update display if true
uint8_t update_display = true;

// Current status (0 inactive, 1 active)
uint8_t state = 0;

// Show "dOnE" message for X seconds if state is 1
uint8_t end_count = END_COUNT;

// Brightness countdown
uint16_t brightness_count = BRIGHTNESS_COUNT;

// Countdown Timer (1sec)
IntervalTimer Timer;
// Brightness Timer (1msec)
IntervalTimer TimerBrightness;

TM1637Display display(DISP_CLK, DISP_DIO);

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};


// Update display and set brightness.
void updateDisplay(uint8_t brightness = BRIGHTNESS_HIGH){
  display.setBrightness(brightness);
  Serial.print(brightness);
  Serial.print(" - ");
  if(end_count == END_COUNT){ // Show current countdown
    display.showNumberDecEx(count,0b11100000,true,4,0);
    Serial.println(count);
  }else{ // Show "dOnE"
    display.setSegments(SEG_DONE);
    Serial.println("dOnE");
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(ENC_01,INPUT_PULLUP);
  pinMode(ENC_02,INPUT_PULLUP);
  pinMode(ENC_SW,INPUT_PULLUP);
  pinMode(LIGHT,OUTPUT);
  pinMode(GEN,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_01), encInt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_SW), updateState, FALLING);
  Timer.priority(0);
  TimerBrightness.priority(1);
  TimerBrightness.begin(countBrightness, 1000);
}

void loop() {
  if(update_display){
    brightness_count = BRIGHTNESS_COUNT;
    updateDisplay(BRIGHTNESS_HIGH);
    update_display = false;
  }
  delay(10);
}

// Encoder interrupt
void encInt(){
  if(debounce_filter_enc<millis()){
    if(digitalRead(ENC_01)==digitalRead(ENC_02)){
      if(state)
        countInc(1);
      else
        countInc(30);
    }else{
      if(state){
        if(count > 1)
          countDec(1);
      }else{
        if(count > 30)
          countDec(30);
      }
    }
    if(!state)
      last_count = count; // Reset to last position
    debounce_filter_enc = millis()+DEBOUNCE_FILTER;
  }
}

void updateState() {
  if(!state){
    // Start countdown
    state = 1;
    Timer.begin(countStep, 1000000);
    brightness_count = BRIGHTNESS_COUNT;
    digitalWrite(LIGHT,HIGH);
    digitalWrite(GEN,HIGH);
  }else{
    // Turn off system and end coundown
    state = 0;
    Timer.end();
    count = last_count;
    update_display = true;
    digitalWrite(LIGHT,LOW);
    digitalWrite(GEN,LOW);
  }
  // Reset end coundown
  end_count = END_COUNT;
}

// Countdown Step
void countStep(){
  if(count){
    // Decrement countdown
    countDec(1);
  }else{
    if(end_count){
      // Show "dOnE" message
      end_count --;
      digitalWrite(GEN,LOW);
    }else{
      // Turn off system and end coundown
      state = 0;
      Timer.end();
      count = last_count;
      end_count = END_COUNT;
      digitalWrite(LIGHT,LOW);
    }
    update_display = true;
    digitalWrite(GEN,LOW);
  }
}

// Increment counter
void countInc(uint8_t i){
  // 100 = 1:00
  if(count<COUNT_MAX){
    if((count+40+i)%100 == 0){
      count += 40+i;
    }else{
      count += i;
    }
  }
  update_display = true;
}

// Decrement counter
void countDec(uint8_t i){
  // 100 = 1:00
  if((count)%100 == 0){
    count -= 40+i;
  }else{
    count -= i;
  }
  update_display = true;
}

// Set brightness to high for X mseconds.
void countBrightness(){
  if(brightness_count){
    brightness_count--;
    if(!brightness_count){
      updateDisplay(BRIGHTNESS_LOW);
    }
  }
}
