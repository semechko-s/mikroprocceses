#include <avr/io.h>

// --- ТВОИ КОНСТАНТЫ (НЕ ТРОГАЕМ) ---
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte colPins[COLS] = {6, 5, 4, 3};
byte rowPins[ROWS] = {A3, A2, A1, A0};

// --- UART ---
#define F_CPU 16000000UL
#define BAUD 9600
#define UBRR_VALUE ((F_CPU/16/BAUD)-1)

void UART_init() {
  UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
  UBRR0L = (unsigned char)UBRR_VALUE;
  UCSR0B = (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_sendChar(char c) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = c;
}

void UART_sendString(const char* str) {
  while (*str) {
    UART_sendChar(*str++);
  }
}

// --- ЗАДЕРЖКА ---
void delay_ms() {
  for (volatile long i = 0; i < 20000; i++);
}

// --- SETUP ---
void setup() {

  // --- PWM (Timer1, OC1A = D9) ---
  DDRB |= (1 << DDB1);

  TCCR1A = (1 << WGM10) | (1 << COM1A1);
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);

  OCR1A = 0;

  // --- UART ---
  UART_init();

  // --- КОЛОНКИ (выход) ---
  DDRD |= (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6);
  PORTD |= (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6);

  // --- СТРОКИ (вход + подтяжка) ---
  DDRC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3));
  PORTC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3);
}

// --- LOOP ---
void loop() {

  for (byte col = 0; col < COLS; col++) {

    // все HIGH
    PORTD |= (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6);

    // текущий LOW
    PORTD &= ~(1 << (PD3 + col));

    delay_ms();

    for (byte row = 0; row < ROWS; row++) {

      if (!(PINC & (1 << row))) {

        char key = keys[row][col];

        
        // --- PWM управление ---
        switch(key){
          case '1': OCR1A = 0;   break;
          case '2': OCR1A = 64;  break;
          case '3': OCR1A = 128;  break;
          case '4': OCR1A = 192;  break;
          case '5': OCR1A = 255; break;
        }

        // --- ВЫВОД В ТЕРМИНАЛ ---
        UART_sendString("Key: ");
        UART_sendChar(key);
        UART_sendString("\r\n");

        delay_ms(); // антидребезг
      }
    }
  }
}
