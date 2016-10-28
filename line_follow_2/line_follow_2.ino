#include <Servo.h>

// Servo speed definitions
#define SERVO_BRAKE 90
#define SERVO_L_FORWARD_MAX 100
#define SERVO_R_FORWARD_MAX 80
#define SERVO_L_INCR_FORWARD 2
#define SERVO_R_INCR_FORWARD -2

#define ERROR_RANGE 15

// Sensor pins
int right_pid = A0; // right center
int left_pid = A1; // left center
int right_turn = 9; // right turn sensor 
int left_turn = 7; // left turn sensor

// Servo objects
Servo left_servo;
Servo right_servo;

// Control variables
float set_point = 0;
float curr_pos = 0;
float error = 0;

// Sensor readings
int right_pid_val = 0; 
int left_pid_val = 0;
int right_turn_val = 0;
int left_turn_val = 0;

// Drive direction commands
int drive_forward = 1;
int drive_left = 0;
int drive_right = 1;
int drive_back = 0;

void setup() {
  
  // Begin Serial monitor
  Serial.begin(9600);

  // Connect servos 
  right_servo.attach(11); 
  left_servo.attach(10);

  // Brake servos
  right_servo.write(SERVO_BRAKE);
  left_servo.write(SERVO_BRAKE);
  
  // Configure left and right digital sensors
  pinMode(right_turn, INPUT);
  pinMode(left_turn, INPUT);

  // Callibrate center sensors for value when both over line
  set_point =  analogRead(left_pid) - analogRead(right_pid);

  // Wait at start for 1 second
  delay(1000);

}

void loop() {
  // Read analog values from two center sensors
  right_pid_val = analogRead(right_pid); //signal from center right sensor
  left_pid_val = analogRead(left_pid); //signal from center left sensor

  curr_pos = left_pid_val - right_pid_val; // Positive position to right of line
  error = curr_pos - set_point;
  
  // Read digital values from left and right turn sensors (Schmitt Triggers)
  right_turn_val = digitalRead(right_turn);
  left_turn_val = digitalRead(left_turn);


  
  // If not at intersection, drive forward 
  if (right_turn_val == 0 && left_turn_val == 0 && drive_forward){
    // Continue to drive forward, making corrections as necessary
    if (abs(error) <= ERROR_RANGE){
      left_servo.write(SERVO_L_FORWARD_MAX);
      right_servo.write(SERVO_R_FORWARD_MAX);
      delay(1);
    }
    // Too right
    else if (error > ERROR_RANGE) {
      // Adjust left
      left_servo.write(SERVO_L_FORWARD_MAX - SERVO_L_INCR_FORWARD);
      right_servo.write(SERVO_R_FORWARD_MAX + SERVO_R_INCR_FORWARD);
      //delay(6);
      //left_servo.write(SERVO_L_FORWARD_MAX + SERVO_L_INCR_FORWARD);
      //delay(1);
    }
    else if (error < ERROR_RANGE) {
      // Adjust right
      left_servo.write(SERVO_L_FORWARD_MAX + SERVO_L_INCR_FORWARD);
      right_servo.write(SERVO_R_FORWARD_MAX - SERVO_R_INCR_FORWARD);
      //delay(6);
      //right_servo.write(SERVO_R_FORWARD_MAX + SERVO_R_INCR_FORWARD);
      //delay(1);
    }
  }
  // Else, make decision to turn left or right
  else if (right_turn_val == 1 && left_turn_val == 1 && drive_right){
    // Make 90 degree right turn

    //drive forward until front sensors are clear of the horizontal tape
    delay(45);


    left_servo.write(SERVO_L_FORWARD_MAX);
    right_servo.write(SERVO_L_FORWARD_MAX);
    delay(200);

    right_turn_val = 0;
    left_turn_val = 0;
    while((abs(error) > 2*ERROR_RANGE) || (right_pid_val<800)){
      left_servo.write(SERVO_L_FORWARD_MAX);
      right_servo.write(SERVO_L_FORWARD_MAX);
      right_pid_val = analogRead(right_pid); //signal from center right sensor
      left_pid_val = analogRead(left_pid); //signal from center left sensor
      curr_pos = left_pid_val - right_pid_val; // Positive position to right of line
      error = curr_pos - set_point;
      Serial.println(error);
      Serial.println(right_turn_val);
      Serial.println(left_turn_val);
      Serial.println();

      delay(5);
    }
    
    left_servo.write(90);
    right_servo.write(90);
    delay(10000);
  }
  else if (right_turn_val == 1 && left_turn_val == 1 && drive_left){
    // Make 90 degree left turn
  }
  else if (right_turn_val == 1 && left_turn_val == 1 && drive_back){
    // Make 180 turn
  }
  delay(10);
}
