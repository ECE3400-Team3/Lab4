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

int error_magnitude = 0;

// Sensor readings
int right_pid_val = 0; 
int left_pid_val = 0;
int right_turn_val = 0;
int left_turn_val = 0;
int at_intersection = 0;


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

  at_intersection = left_turn_val && right_turn_val;
  
  if (at_intersection){
    if (drive_right){
      // Make 90 degree right turn
      left_servo.write(SERVO_L_FORWARD_MAX);
      right_servo.write(SERVO_R_FORWARD_MAX);
      //drive forward until front sensors are clear of the horizontal tape
      delay(45);
 
      left_servo.write(SERVO_L_FORWARD_MAX);
      right_servo.write(SERVO_L_FORWARD_MAX);
      delay(300);
      right_pid_val = analogRead(right_pid); //signal from center right sensor
      left_pid_val = analogRead(left_pid); //signal from center left sensor
      curr_pos = left_pid_val - right_pid_val; // Positive position to right of line
      error = curr_pos - set_point;
      right_turn_val = 0;
      left_turn_val = 0;
      while((abs(error) < ERROR_RANGE) || (right_pid_val<900)){
      left_servo.write(SERVO_L_FORWARD_MAX);
      right_servo.write(SERVO_L_FORWARD_MAX);
      right_pid_val = analogRead(right_pid); //signal from center right sensor
      left_pid_val = analogRead(left_pid); //signal from center left sensor
      curr_pos = left_pid_val - right_pid_val; // Positive position to right of line
      error = curr_pos - set_point;
      Serial.println(error);
      Serial.println(right_pid_val);
      Serial.println(left_pid_val);
      Serial.println();
      delay(5);
      }
    
      left_servo.write(90);
      right_servo.write(90);
      delay(1000);
    }
    
  }
  else{
    if(drive_forward){
      // Continue to drive forward, making corrections as necessary
      if (abs(error) <= ERROR_RANGE){
      left_servo.write(SERVO_L_FORWARD_MAX);
      right_servo.write(SERVO_R_FORWARD_MAX);
      delay(1);
      }
      // Too right
      else if (error > ERROR_RANGE) {
      // Adjust left
      error_magnitude = error/ERROR_RANGE;
      left_servo.write(SERVO_L_FORWARD_MAX - error_magnitude*SERVO_L_INCR_FORWARD);
      right_servo.write(SERVO_R_FORWARD_MAX + error_magnitude*SERVO_R_INCR_FORWARD);
      //delay(6);
      //left_servo.write(SERVO_L_FORWARD_MAX + SERVO_L_INCR_FORWARD);
      //delay(1);
      }
      else if (error < -ERROR_RANGE) {
      // Adjust right
      error_magnitude = -error/ERROR_RANGE;
      left_servo.write(SERVO_L_FORWARD_MAX + error_magnitude*SERVO_L_INCR_FORWARD);
      right_servo.write(SERVO_R_FORWARD_MAX - error_magnitude*SERVO_R_INCR_FORWARD);
      //delay(6);
      //right_servo.write(SERVO_R_FORWARD_MAX + SERVO_R_INCR_FORWARD);
      //delay(1);
      }
    }

  delay(10);
  }
}
