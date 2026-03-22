#include <avr/io.h>
#include <avr/interrupt.h>

// --- Клавиатура 4x4 ---
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte colPins[COLS] = {6,5,4,3};
byte rowPins[ROWS] = {A3,A2,A1,A0};

// --- 7-сегментный дисплей ---
byte segPins[7] = {8,9,10,11,12,13,7};
const byte DIG1 = 2;   // десятки
const byte DIG2 = A5;  // единицы

const byte digits[10] = { 
  0b00111111,0b00000110,0b01011011,0b01001111,
  0b01100110,0b01101101,0b01111101,0b00000111,
  0b01111111,0b01101111
};

// --- Зуммер ---
const byte BUZZER = A4;

// --- Таймер ---
volatile int timeLeft = 0;
volatile bool timerRunning = false;

// --- Ввод ---
bool inputActive = true;
byte tensDigit = 0;
byte unitsDigit = 0;
bool hasFirstDigit = false;

// --- Setup ---
void setup(){
  Serial.begin(9600);

  for(byte i=0;i<COLS;i++){
    pinMode(colPins[i],OUTPUT);
    digitalWrite(colPins[i],HIGH);
  }
  for(byte i=0;i<ROWS;i++){
    pinMode(rowPins[i],INPUT_PULLUP);
  }

  for(byte i=0;i<7;i++){
    pinMode(segPins[i],OUTPUT);
    digitalWrite(segPins[i],HIGH);
  }

  pinMode(DIG1,OUTPUT);
  pinMode(DIG2,OUTPUT);
  digitalWrite(DIG1,HIGH);
  digitalWrite(DIG2,HIGH);

  pinMode(BUZZER,OUTPUT);
  digitalWrite(BUZZER,LOW);

  TCCR1A = 0;
  TCCR1B = (1<<WGM12)|(1<<CS12)|(1<<CS10);
  OCR1A = 15624;
  TIMSK1 = (1<<OCIE1A);
  sei();
}

// --- Таймер ---
ISR(TIMER1_COMPA_vect){
  if(timerRunning && timeLeft>0){
    timeLeft--;
  }
}

// --- Клавиатура ---
char scanKey(){
  for(byte col=0;col<COLS;col++){
    digitalWrite(colPins[col],LOW);
    for(byte row=0;row<ROWS;row++){
      if(digitalRead(rowPins[row])==LOW){
        char k = keys[row][col];
        delay(300);
        digitalWrite(colPins[col],HIGH);
        return k;
      }
    }
    digitalWrite(colPins[col],HIGH);
  }
  return 0;
}

// --- Вывод цифры ---
void showDigit(byte num, byte digPin) {
  for(byte i=0; i<7; i++) digitalWrite(segPins[i], HIGH);
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);

  digitalWrite(digPin, LOW);

  for(byte i=0;i<7;i++){
    bool state = digits[num] & (1<<i);
    digitalWrite(segPins[i], !state);
  }
}

// --- Мультиплекс ---
void displayDigits(byte tens, byte units){
  showDigit(tens, DIG1);
  delay(5);
  showDigit(units, DIG2);
  delay(5);
}

// --- Зуммер ---
void buzzerOn(){ digitalWrite(BUZZER,HIGH); }
void buzzerOff(){ digitalWrite(BUZZER,LOW); }

// --- LOOP ---
void loop(){
  char key = scanKey();

  // --- ВВОД КАК В ТВОЁМ C-КОДЕ ---
  if(inputActive && key){
    if(key>='0' && key<='9'){
      byte digit = key - '0';

      if(!hasFirstDigit){
        unitsDigit = digit;
        tensDigit = 0;
        hasFirstDigit = true;
      } else {
        tensDigit = unitsDigit;
        unitsDigit = digit;
      }
    }

    if(key=='*' && hasFirstDigit){
      timeLeft = tensDigit*10 + unitsDigit;
      timerRunning = true;
      inputActive = false;
    }
  }

  // --- Таймер ---
  if(timerRunning){
    byte t = timeLeft % 10;
    byte u = timeLeft / 10;

    displayDigits(t, u);

    if(timeLeft==0){
      buzzerOn();
      delay(2000);
      buzzerOff();

      timerRunning=false;
      inputActive=true;
      tensDigit=0;
      unitsDigit=0;
      hasFirstDigit=false;
    }
  }
  else{
    if(tensDigit == 0){
      displayDigits(unitsDigit, 0);
    } else {
      displayDigits(unitsDigit, tensDigit);
    }
  }
}
