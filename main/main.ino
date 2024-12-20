//*********** Driver pins ***********
#define M1A 2 // M1 controls the elecctromagnets
#define M1B 3
#define M2A 4 // M2 controls the LED strip
#define M2B 5
//***********************************
#define potPin A1

const float minFrequency = 10; // min frequency in Hz
const float maxFrequency = 12; // max frequency in Hz

float oscFrequency = 0; // initial frequebcy 
unsigned long oscPeriod = 0;

unsigned long lastToggleTimeSolenoid = 0; 
unsigned long lastToggleTimeLED = 0;
bool solenoidState = false; 
bool ledState = false;  
const float phaseShift = 0.25; 

void setup(){
  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  pinMode(potPin, INPUT);
}

void loop(){
  int potVal = analogRead(potPin);
  oscFrequency = map(potVal, 0, 1023, minFrequency * 10, maxFrequency * 10) / 10.0;

  oscPeriod = 1000 / (oscFrequency * 2);
 
  unsigned long currentTime = millis();

  if (currentTime - lastToggleTimeSolenoid >= oscPeriod) {
    lastToggleTimeSolenoid = currentTime; 

    solenoidState = !solenoidState;
    if (solenoidState) {
      digitalWrite(M1A, HIGH);
      digitalWrite(M1B, LOW);
    } else {
      digitalWrite(M1A, LOW);
      digitalWrite(M1B, HIGH);
    }
  }

  if (currentTime - lastToggleTimeLED >= (oscPeriod + oscPeriod * phaseShift)) {
    lastToggleTimeLED = currentTime; 

    ledState = !ledState;
    if (ledState) {
      digitalWrite(M2A, HIGH);
      digitalWrite(M2B, LOW);
    } else {
      digitalWrite(M2A, LOW);
      digitalWrite(M2B, HIGH);
    }
  }
}

