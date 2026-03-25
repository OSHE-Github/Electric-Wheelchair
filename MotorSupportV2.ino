/*TODO:
Modify to work with analogWrite to get PWM working
Label things more
Implement buzzer code do something fun with it
Test on Thursday
*/


// Reads four analog direction inputs and smoothly drives two motors via DRV8871 boards.

// === Analog input pins for direction control ===
const int leftPin = 32; //THIS GOT CHANGED PLEASE MOVE WIRE PIN 35 DOESN'T WORK AS A INPUT_PULLDOWN
const int rightPin = 33;
const int backPin = 26;
const int forwardPin = 14;

// === Motor driver pins ===
// Verify wire locations
const int A_IN1 = 15;  // Right motor IN1 Backward
const int A_IN2 = 2;   // Right motor IN2 Forward
const int B_IN1 = 18;  // Left motor IN1 Forward
const int B_IN2 = 19;  // Left motor IN2 Backward

const int buzzerPin = 21; //Can be switched

//Curent speeds for motor channel
int currentLeftFwd  = 0;
int currentLeftRev  = 0;
int currentRightFwd = 0;
int currentRightRev = 0;

// === Constants ===
const int RAMP_STEPS = 20;  // Number of ramp increments (Controls acceleration)

void setup() {
  //Keeps pins at 0 when no input
  pinMode(forwardPin, INPUT_PULLDOWN);
  pinMode(backPin, INPUT_PULLDOWN);
  pinMode(leftPin, INPUT_PULLDOWN);
  pinMode(rightPin, INPUT_PULLDOWN);

  pinMode(buzzerPin, OUTPUT);

//Set pin resolutions to 12-bits
  analogWriteResolution(A_IN1, 12);
  analogWriteResolution(A_IN2, 12);
  analogWriteResolution(B_IN1, 12);
  analogWriteResolution(B_IN2, 12);

  //Optional for changing frequency of outputs
  //analogWriteFrequency(); //Output forward
}

// === Helper: smooth ramp toward a target speed ===
int rampSpeed(int current, int target, int step) {
  if (current < target) {
    current += step;
    if (current > target) current = target; // Don't overshoot the target
  } else if (current > target) {
    current -= step;
    if (current < target) current = target; // Don't undershoot the target
  }
  return current;
}

void loop() {
  
  //Read input from joystick
  int valForward  = digitalRead(forwardPin) * 4095; 
  int valBackward = digitalRead(backPin) * 4095;
  int valLeft     = digitalRead(leftPin) * 4095;
  int valRight    = digitalRead(rightPin) * 4095;

  //Checks if reversing
  bool isReversing = (valBackward > 0);
  //Sounds the buzzer
  //May need to use tone function if it is a passive buzzer
  if(isReversing){
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }

  //Find target speed
  int targetLeftFwd  = constrain(valForward + valRight, 0, 4095);
  int targetLeftRev  = constrain(valBackward + valLeft, 0, 4095);
  int targetRightFwd = constrain(valForward + valLeft, 0, 4095);
  int targetRightRev = constrain(valBackward + valRight, 0, 4095);

  //Ramp current speed to target speed
  currentLeftFwd  = rampSpeed(currentLeftFwd, targetLeftFwd, rampStep);
  currentLeftRev  = rampSpeed(currentLeftRev, targetLeftRev, rampStep);
  currentRightFwd = rampSpeed(currentRightFwd, targetRightFwd, rampStep);
  currentRightRev = rampSpeed(currentRightRev, targetRightRev, rampStep);

  //Send signal to motors
  analogWrite(B_IN1, currentLeftFwd);
  analogWrite(B_IN2, currentLeftRev);
  analogWrite(A_IN2, currentRightFwd);
  analogWrite(A_IN1, currentRightRev);

  delay(10);
}
