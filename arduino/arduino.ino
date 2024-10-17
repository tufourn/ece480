#define DROPS_PER_INCH 96
#define INCHES_PER_REV 1.5748 // 40 mm per revolution
#define STEPS_PER_REV 3200    // 200 full steps * 16 microsteps
#define STEPS_PER_DROP 21     // (1/96)*(1/1.5748)*3200 = 21.17
#define NOZZLE_COUNT 1

#define MOTOR_X_DIR_PIN 2
#define MOTOR_X_STEP_PIN 3
#define MOTOR_X_LIMIT_PIN 6
#define MOTOR_Y_DIR_PIN 3
#define MOTOR_Y_STEP_PIN 4
#define MOTOR_Y_LIMIT_PIN 7

enum CommandMode {
  INSTRUCTION,
  MASK1,
  MASK2,
};

struct StepperMotor {
  int dirPin;
  int stepPin;
  int limitPin;
};

CommandMode cmdMode = INSTRUCTION;
StepperMotor motorX = {MOTOR_X_DIR_PIN, MOTOR_X_STEP_PIN, MOTOR_X_LIMIT_PIN};
StepperMotor motorY = {MOTOR_Y_DIR_PIN, MOTOR_Y_STEP_PIN, MOTOR_Y_LIMIT_PIN};

int stepsTaken = 0;

// move stepper
void step(const StepperMotor& motor, int stepCount) {
  if(stepCount >= 0) {
    digitalWrite(motor.dirPin, 0);
  } else {
    digitalWrite(motor.dirPin, 1);
  }
  for(int i = 0; i < stepCount; i++) {
    digitalWrite(motor.stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(motor.stepPin, LOW);
    delayMicroseconds(1000);
  }
}

// move one dot width to right
// track step count for return
void stepDrop() {
  step(motorX, STEPS_PER_DROP);
  stepsTaken += STEPS_PER_DROP;
}

// goto the begin of current line
void gotoBeginLine() {
  step(motorX, stepsTaken*-1);
}

// move stepper y 1 line above (dot height) * nozzleCount
void gotoNextLine() {
  step(motorY, STEPS_PER_DROP*NOZZLE_COUNT);
}

// dispense ink based on bitmask
// when last mask done, call step() and change cmdMode to INSTRUCTION
void dispense(unsigned char mask) {
  if (cmdMode == MASK1) {
    /* TODO: dispense first set of nozzles */
    cmdMode = MASK2;
  }
  if (cmdMode == MASK2) {
    /* TODO: dispense second set of nozzles */
    cmdMode = INSTRUCTION;
    stepDrop();
  }
}

void setup() {
  pinMode(motorX.dirPin, OUTPUT);
  pinMode(motorX.stepPin, OUTPUT);
  pinMode(motorY.dirPin, OUTPUT);
  pinMode(motorY.stepPin, OUTPUT);
  
  Serial.begin(115200);
  while (Serial.available() <= 0) {
    Serial.print('X');
    delay(100);
  }
}

void loop() {
  if (Serial.available() > 0) {
    unsigned char cmd = Serial.read();
    Serial.print('R'); // let host know byte has been received
    if (cmdMode == INSTRUCTION) {
      switch (cmd) {
        case 'D':
          cmdMode = MASK1;
          break;
        case 'R':
          gotoBeginLine();
          break;
        case 'N':
          gotoNextLine();
          break;
        default:
          break;
      }
    } else {
      dispense(cmd);
    }
  }
}
