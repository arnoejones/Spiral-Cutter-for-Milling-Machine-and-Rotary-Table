/* 
The encoder portion of this code is lifted from MoThunderz at https://github.com/mo-thunderz/RotaryEncoder.  
I learned a lot from him.
*/

#include <LiquidCrystal_I2C.h> 
#include <Wire.h>

// Define rotary encoder pins
#define ENC_A 2
#define ENC_B 3
#define DirPin 6
#define PulsePin 7
#define StepsToTake 4  // This is the "factor" in determining the ratio of input to output
//for 0.33" in the x direction, the turntable will turn 90º giving me my 23º spiral

unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int Direction = 0;
volatile int counter = 0;
volatile int stepsToTake = StepsToTake;
LiquidCrystal_I2C lcd(0x27,20,4); //i2c addr, 20 chars, 4 lines
void setup() {

  // Set encoder pins and attach interrupts
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(DirPin, OUTPUT);
  pinMode(PulsePin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, HIGH);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, HIGH);

  // Start the serial monitor to show output
  Serial.begin(115200);
  lcd.init();
  lcd.init(); //dunno why I have to do this twice
  lcd.backlight();
}

void loop() {
  static int lastCounter = 0;
  // If count has changed print the new value to serial
  if(counter != lastCounter){
    lastCounter = counter;
  }
  lcd.setCursor(0,0);
  lcd.print("Encoder: ");
  lcd.setCursor(9,0);
  lcd.print(counter);
  lcd.print("         "); 
  lcd.setCursor(14, 0);
  lcd.print(int(counter * 0.6));  //convert steps to degrees
  lcd.setCursor(18, 0);
  lcd.print("\xDF");
  
  lcd.setCursor(0,2);
  lcd.print("Stepper: ");  
  lcd.setCursor(9,2);  
  lcd.print(counter * 3);
  lcd.print("      "); 
  lcd.setCursor(14, 2);
  /*
  Encoder to Stepper is 600:200.  To get stepper value
  multiply 3.  Multiply by 0.6 to convert to degrees
  then divide by turntable ratio, which is 40:1
  */
  lcd.print(int(counter * 3 * 0.6 / 40)); //convert steps to degrees 
  lcd.setCursor(18, 2);
  lcd.print("\xDF");

  //lcd.print("      ");  
}

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    Direction = 1;
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
     // changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
    sendPulseToStepper(Direction);
  }
  else if( encval < -3 ) {        // Four steps backward
    Direction = 0;
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      //changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
    sendPulseToStepper(Direction);
  }
} 
void sendPulseToStepper(bool Direction){
  digitalWrite(DirPin, Direction); 
  //4 times equals one step, i.e., 1.8º 
  for (int x = 0; x < StepsToTake; x++){
    digitalWrite(PulsePin, HIGH);
    delayMicroseconds(10);
    digitalWrite(PulsePin, LOW);
    delayMicroseconds(10);        
    digitalWrite(PulsePin, HIGH);
    delayMicroseconds(10);
    digitalWrite(PulsePin, LOW);
    delayMicroseconds(10);    
    digitalWrite(PulsePin, HIGH);
    delayMicroseconds(10);
    digitalWrite(PulsePin, LOW);
    delayMicroseconds(10);    
    digitalWrite(PulsePin, HIGH);
    delayMicroseconds(10);
    digitalWrite(PulsePin, LOW);
    delayMicroseconds(10); 
  }       
}