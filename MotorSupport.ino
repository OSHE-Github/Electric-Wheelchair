//4-directional joystick program

//DON'T USE PINS
/*
No input: 0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 34, 35, 36, 39
*/

// === Pin Locations ===
const int leftPin = 4;    // Yellow wire
const int rightPin = 19;   // Blue wire
const int backPin = 21;    // Black wire
const int forwardPin = 18;  // Red wire

// === Motor driver pins ===
const int A_IN1 = 13;  // Right motor IN1 Backward
const int A_IN2 = 27;  // Right motor IN2 Forward
const int B_IN1 = 25;  // Left motor IN1 Forward
const int B_IN2 = 33;  // Left motor IN2 Backward
const int enablePin = 32;

// === Buzzer Pin ===
const int buzzerPin = 26;

// === Constants ===
const int MAX_SPEED = 3000;
const int TURN_SPEED_OUTSIDE = 2600;
const int TURN_SPEED_INSIDE  = -2400;
const int RAMP_STEP = 25;          // Number of ramp increments (Controls acceleration)
const long buzzerInterval = 300;  // How fast we "BeepBeepBeep"

// === Kickstart Constants ===
const int  KICKSTART_PWM = 3800;      
const int MIN_SUSTAIN_PWM = 1400;     // The baseline speed to drop to after the kickstart finishes
const int KICKSTART_DURATION = 250;  // Milliseconds to apply the high torque

// === PWM Settings ===
const int freq = 5000;
const int resolution = 12;

// === Intermediate Variables ===
int currentLeft = 0;
int currentRight = 0;
int targetLeft = 0;
int targetRight = 0;

unsigned long lastToggle = 0;
bool buzzerIsOn = false;
bool buzzerState = false;

// === Kickstart Flags === 
unsigned long leftKickstartTimer = 0;
unsigned long rightKickstartTimer = 0;
bool isLeftKickstarting = false;
bool isRightKickstarting = false;

void setup() {
  //Set Controller Pins
  pinMode(forwardPin, INPUT_PULLDOWN);
  pinMode(backPin, INPUT_PULLDOWN);
  pinMode(leftPin, INPUT_PULLDOWN);
  pinMode(rightPin, INPUT_PULLDOWN);

  //Buzzer Pin
  ledcAttach(buzzerPin, 2000, 8);

  //Enable Pin High
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);

  // Setup PWM channels
  ledcAttach(A_IN2, freq, resolution);
  ledcAttach(A_IN1, freq, resolution);
  ledcAttach(B_IN1, freq, resolution);
  ledcAttach(B_IN2, freq, resolution);
}

//Ramp Helper Method
int rampSpeed(int current, int target, int step) {
  if (current < target) {
    current += step;
    if (current > target) current = target;  // Don't overshoot the target
  } else if (current > target) {
    current -= step;
    if (current < target) current = target;  // Don't undershoot the target
  }
  return current;
}

// === Translation Functions ===

//Left motor driving function
void driveLeftMotor(int speed) {
  if (speed > 0) {
    ledcWrite(B_IN1, speed);
    ledcWrite(B_IN2, 0);
  } else if (speed < 0) {
    ledcWrite(B_IN1, 0);
    ledcWrite(B_IN2, abs(speed));
  } else {
    ledcWrite(B_IN1, 0);
    ledcWrite(B_IN2, 0);
  }
}

//Right motor driving function
void driveRightMotor(int speed) {
  if (speed > 0) {
    ledcWrite(A_IN2, speed);
    ledcWrite(A_IN1, 0);
  } else if (speed < 0) {
    ledcWrite(A_IN2, 0);
    ledcWrite(A_IN1, abs(speed));
  } else {
    ledcWrite(A_IN2, 0);
    ledcWrite(A_IN1, 0);
  }
}

void loop() {

  bool forward = digitalRead(forwardPin);
  bool back = digitalRead(backPin);
  bool left = digitalRead(leftPin);
  bool right = digitalRead(rightPin);

/*
  //TEST ALL DIRECTIONS
  if(forward && !back){
    ledcWrite(A_IN2, MAX_SPEED);
    ledcWrite(A_IN1, 0);
    ledcWrite(B_IN2, 0);
    ledcWrite(B_IN1, MAX_SPEED);
  } else if(back && !forward){
    ledcWrite(A_IN2, 0);
    ledcWrite(A_IN1, MAX_SPEED);
    ledcWrite(B_IN2, MAX_SPEED);
    ledcWrite(B_IN1, 0);
  } else if(right){
    ledcWrite(A_IN2, 0);
    ledcWrite(A_IN1, MAX_SPEED);
    ledcWrite(B_IN2, 0);
    ledcWrite(B_IN1, MAX_SPEED);
  } else if(left){
    ledcWrite(A_IN2, MAX_SPEED);
    ledcWrite(A_IN1, 0);
    ledcWrite(B_IN2, MAX_SPEED);
    ledcWrite(B_IN1, 0);
  } else{
    ledcWrite(A_IN2, 0);
    ledcWrite(A_IN1, 0);
    ledcWrite(B_IN2, 0);
    ledcWrite(B_IN1, 0);
  }
*/

  //Default to zero
  targetLeft = 0;
  targetRight = 0;

  //Joystick directions
  //Forward
  if (forward && !back) {
    targetLeft = MAX_SPEED;
    targetRight = MAX_SPEED;
  }
  //Backward
  else if (back && !forward) {
    targetLeft = -MAX_SPEED;
    targetRight = -MAX_SPEED;
    buzzerIsOn = true;
  }
  //Right
  else if (right && !left) {
    targetLeft = TURN_SPEED_OUTSIDE;
    targetRight = TURN_SPEED_INSIDE;
  }
  //Left
  else if (left && !right) {
    targetLeft = TURN_SPEED_INSIDE;
    targetRight = TURN_SPEED_OUTSIDE;
  }
  //Default to 0
  else {
    targetLeft = 0;
    targetRight = 0;
    buzzerIsOn = false;
  }

  //SPEED SYNCHRONIZATION
  if(targetLeft == targetRight && targetLeft != 0){
    if(abs(currentLeft - currentRight) > RAMP_STEP){

      int midPoint = (currentLeft + currentRight) / 2;

      targetLeft = midPoint;
      targetRight = midPoint;

    }
  }

//Kickstart logic
// LEFT MOTOR
  if (targetLeft != 0 && currentLeft == 0) {
    isLeftKickstarting = true;
    leftKickstartTimer = millis();
    // Jumpstart the ramp baseline
    currentLeft = (targetLeft > 0) ? MIN_SUSTAIN_PWM : -MIN_SUSTAIN_PWM;
  }
  if (targetLeft == 0) {
    isLeftKickstarting = false;
  }

  // RIGHT MOTOR
  if (targetRight != 0 && currentRight == 0) {
    isRightKickstarting = true;
    rightKickstartTimer = millis();
    // Jumpstart the ramp baseline
    currentRight = (targetRight > 0) ? MIN_SUSTAIN_PWM : -MIN_SUSTAIN_PWM;
  }
  if (targetRight == 0) {
    isRightKickstarting = false;
  }

  //Calculate ramped speed
  currentLeft = rampSpeed(currentLeft, targetLeft, RAMP_STEP);
  currentRight = rampSpeed(currentRight, targetRight, RAMP_STEP);

//Kickstart Override
  int finalLeftOutput = currentLeft;
  int finalRightOutput = currentRight;

  if (isLeftKickstarting) {
    if (millis() - leftKickstartTimer < KICKSTART_DURATION) {
      int leftKickValue = map(abs(targetLeft), 0, MAX_SPEED, 0, KICKSTART_PWM);
      finalLeftOutput = (targetLeft > 0) ? leftKickValue : -leftKickValue;
    } else {
      isLeftKickstarting = false; // Timer finished
    }
  }

  if (isRightKickstarting) {
    if (millis() - rightKickstartTimer < KICKSTART_DURATION) {
      int rightKickValue = map(abs(targetRight), 0, MAX_SPEED, 0, KICKSTART_PWM);
      finalRightOutput = (targetRight > 0) ? rightKickValue : -rightKickValue;
    } else {
      isRightKickstarting = false; // Timer finished
    }
  }

  //Send speed to motor driving function
  driveLeftMotor(finalLeftOutput);
  driveRightMotor(finalRightOutput);

  //Reverse Buzzer Function
  if (buzzerIsOn) {
    if (millis() - lastToggle > buzzerInterval) {
      lastToggle = millis();
      buzzerState = !buzzerState;

      if (buzzerState) {
        ledcWriteTone(buzzerPin, 2000);
      } else {
        ledcWriteTone(buzzerPin, 0);
      }
    }
  } else {
    ledcWriteTone(buzzerPin, 0); // ensure off
  }

  delay(20);
}