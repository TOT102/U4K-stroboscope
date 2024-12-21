#define DEF_OFFSET 50
#define EXPO 400 // how long the LED is enabled in microseconds
#define PERIOD 5000 // priod of flickering in microseconds
#define SET_TIMER 500

#define M1A 5
#define M1B 6
#define M2A 9
#define M2B 10
#define POT_PIN A0

volatile bool solenoidState = false;
volatile bool ledState = false;
volatile uint16_t oscPeriod = PERIOD;
volatile uint16_t lightTimer = EXPO;
volatile uint16_t flashDelay = PERIOD * 2 + DEF_OFFSET;
uint32_t lastPotRead = 0;

void setup() {
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  digitalWrite(M1A, LOW);
  digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW);
  digitalWrite(M2B, LOW);

//*********** TIMERS **********
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11); // CTC rejim s 8 scaler, da se chisti kat se napulni
  OCR1A = PERIOD;
  TIMSK1 |= (1 << OCIE1A);

  TCCR3A = 0;
  TCCR3B = (1 << WGM32) | (1 << CS31);
  OCR3A = EXPO;
  TIMSK3 |= (1 << OCIE3A);
//*****************************

  setADCrate(2); // acceleration of analogRead ama neshto ne bachka
}

void loop() {
  // timer of pot reading
  if (millis() - lastPotRead >= SET_TIMER) {
    lastPotRead = millis();
    uint16_t potValue = readADC(POT_PIN);
    float oscFrequency = map(potValue, 0, 1023, 10 * 10, 12 * 10) / 10.0;
    noInterrupts();
    oscPeriod = 1000000 / (oscFrequency * 2);
    flashDelay = oscPeriod * 2 + DEF_OFFSET;
    OCR1A = oscPeriod;
    OCR3A = EXPO;
    interrupts();
  }
}

ISR(TIMER1_COMPA_vect) {
  solenoidState = !solenoidState;
  if (solenoidState) {
    PORTC |= (1 << 6);
    PORTD &= ~(1 << 7);
  } else {
    PORTC &= ~(1 << 6);
    PORTD |= (1 << 7);
  }
}

ISR(TIMER3_COMPA_vect) {
  static bool flashState = false;

  flashState = !flashState;
  if (flashState) {
    // fast analogue of digitalWrite
    PORTB |= (1 << 5);
    PORTB &= ~(1 << 6);
    OCR3A = EXPO;
  } else {
    // fast analogue of digitalWrite
    PORTB &= ~(1 << 5);
    PORTB |= (1 << 6);
    OCR3A = flashDelay - EXPO;
  }
}

void setADCrate(byte mode) {
  ADCSRA = (ADCSRA & ~0x07) | (mode & 0x07);
}

uint16_t readADC(uint8_t pin) { // neshto ne bachka trqbwa da se testwa
  ADMUX = (ADMUX & 0xF0) | (pin & 0x0F);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}
