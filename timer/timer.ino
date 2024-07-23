#include <Arduino.h>
#include <TM1637Display.h>

#define INTERRUPT 2

// Module A
#define DIO_A 3
#define CLK_A 4
// Module B
#define DIO_B 5
#define CLK_B 6

// Buttons
#define BTN_A 9
#define BTN_B 10
#define BTN_C 12
#define BTN_D 11


#define BRIGH 0x0f
#define MAX_LEAPS 16

TM1637Display da(CLK_A, DIO_A);
TM1637Display db(CLK_B, DIO_B);

unsigned long start = 0;
bool running = false;
unsigned long elapsed = 0;
unsigned long saved_leaps[MAX_LEAPS];
int leapCounter = 0;
int showLeap = 0;

void resetLeaps() {
  for (int i = 0; i < MAX_LEAPS; i++) {
    saved_leaps[i] = 0;
  }
  leapCounter = 0;
  showLeap = 0;
}
void insertLeap(unsigned long leap) {
    // Check if there is space in the array
    if (leapCounter >= MAX_LEAPS) return;

    // Shift all elements to the right
    for (int i = leapCounter; i > 0; i--) {
        saved_leaps[i] = saved_leaps[i - 1];
    }

    // Insert the new element at the front
    saved_leaps[0] = leap;

    // Increment the size
    leapCounter++;
}

void startTimer() {
  start = millis();
  running = true;
}
void pauseTimer() {
  running = false;
  elapsed += (millis() - start);
  showLeap = 0;
}
void resetTimer() {
  elapsed = 0;
  running = false;
  resetLeaps();
}
void saveLeap() {
  insertLeap(elapsed + (millis() - start));
}
void leapUp(){
  if(showLeap >= leapCounter) return;
  showLeap++;
}
void leapDown(){
  if(showLeap <= 0) return;
  showLeap--;
}


bool btn_a, btn_b, btn_c, btn_d = false;
void checkButtons() {
  // BTN A
  if(digitalRead(BTN_A) == LOW && !btn_a) {
    btn_a = true;
    Serial.println("A");
    // pressed
    if(running) pauseTimer();
    else startTimer();
  } 
  else if (digitalRead(BTN_A) == HIGH && btn_a) {
    btn_a = false;

    // released
  }

  // BTN B
  if(digitalRead(BTN_B) == LOW && !btn_b) {
    btn_b = true;
    Serial.println("B");
    // pressed
    if(running) saveLeap();
    else resetTimer();
  } 
  else if (digitalRead(BTN_B) == HIGH && btn_b) {
    btn_b = false;
    // released
  }

  // BTN C
  if(digitalRead(BTN_C) == LOW && !btn_c) {
    btn_c = true;
    Serial.println("C");
    // pressed
    if(!running) leapUp();
  } else if (digitalRead(BTN_C) == HIGH && btn_c) {
    btn_c = false;
    // released
  }

  // BTN D
  if(digitalRead(BTN_D) == LOW && !btn_d) {
    btn_d = true;
    Serial.println("D");
    // pressed
  } else if (digitalRead(BTN_D) == HIGH && btn_d) {
    btn_d = false;
    // released
    if(!running) leapDown();
  }
}

unsigned long last_interrupt_time = 0;

void interrupt_handler()
{
  Serial.println("INTER");
  
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if ((millis() - last_interrupt_time) > 50) 
  {
    checkButtons();
  }
  last_interrupt_time = millis();
}

void showTime(unsigned long mils, bool dots = true) {
  // milliseconds
  da.showNumberDec((mils % 1000) * 10, true, 4, 0);

  int sec = (mils / 1000) % 60;
  int min = ((mils / 60000) % 60);

  db.showNumberDec(sec, true, 2, 2);

  if(dots) db.showNumberDecEx(min, 0x40, true, 2, 0);
  else db.showNumberDecEx(min, 0x00, true, 2, 0);
}

void setup()
{
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(BTN_D, INPUT_PULLUP);
  pinMode(INTERRUPT, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(INTERRUPT), interrupt_handler, CHANGE);

  da.setBrightness(BRIGH);
  da.clear();
  
  db.setBrightness(BRIGH);
  db.clear();
 
  start = millis();
  elapsed = 0;
  running = false;
  Serial.begin(9600);

  resetLeaps();
}

void loop() {
  // checkButtons();
  if(running) {
    showTime((elapsed + (millis() - start)));
  } else {
    if(showLeap == 0) {
      showTime(elapsed);
    } else {
      showTime(saved_leaps[showLeap - 1]);
    }
  }
}


