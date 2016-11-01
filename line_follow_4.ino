#include <Servo.h>

// Servo speed definitions
#define SERVO_BRAKE 90
#define SERVO_L_FORWARD_MAX 100.0
#define SERVO_R_FORWARD_MAX 80.0
#define SERVO_L_INCR_FORWARD 1.0
#define SERVO_R_INCR_FORWARD -1.0

#define ERROR_RANGE 40

// Sensor pins
int right_pid = A0; // right center
int left_pid = A1; // left center
int right_turn = A2; // right turn sensor 
int left_turn = A3; // left turn sensor

// Servo objects
Servo left_servo;
Servo right_servo;

// Control variables
float error = 0;
float error_magnitude = 0;

// Sensor readings
int right_pid_val = 0; 
int left_pid_val = 0;
int right_turn_val = 0;
int left_turn_val = 0;
int at_intersection = 0;

// Servo turn values
float servo_turn_value[] = {SERVO_R_FORWARD_MAX,0,SERVO_L_FORWARD_MAX, SERVO_L_FORWARD_MAX};
int servo_turn_delays[] = {300,0,300,700};

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

  // Wait at start for 1 second
  delay(1000);
}

void loop() {
  
}

//function move() is called from the navigation code after an intersection 
//has been reached and the decision of what direction to go has been made
//takes in a direction to move one block and 
//a pointer to the array that stores wall sensor values (this probably doesn't need to be an argument, could be global)
//wall sensor values are modified when the robot reaches the proscribed intersection
//direction: 0 if left, 1 if forward, 2 if right, 3 if 180

void move(int direction, byte *wall_array){

  //drive forward until front sensors are clear of the horizontal tape
  left_servo.write(SERVO_L_FORWARD_MAX);
  right_servo.write(SERVO_R_FORWARD_MAX);     
  delay(60);

  //turn if requested
  if (direction != 1){
    left_servo.write(servo_turn_value[direction]);
    right_servo.write(servo_turn_value[direction]);
    delay(servo_turn_delays[direction]);
    right_pid_val = analogRead(right_pid); //signal from center right sensor
    left_pid_val = analogRead(left_pid); //signal from center left sensor

    while((right_pid_val<800) || (left_pid_val<800)){
        right_pid_val = analogRead(right_pid); //signal from center right sensor
        left_pid_val = analogRead(left_pid); //signal from center left sensor
        }
    //pause momentarily to reduce jitter post turn
    left_servo.write(SERVO_BRAKE);
    right_servo.write(SERVO_BRAKE);
    delay(100);
    //drive forward until tape is cleared
    left_servo.write(SERVO_L_FORWARD_MAX);
    right_servo.write(SERVO_R_FORWARD_MAX);     
    delay(70);
  }

  //after turn is complete, drive forward to the next intersection, making corrections as necessary
  while(right_turn_val<700 || left_turn_val<700){
    //reset intersection variables
    right_turn_val = 0;
    left_turn_val = 0;
    // Read analog values from two center sensors
    right_pid_val = analogRead(right_pid); //signal from center right sensor
    left_pid_val = analogRead(left_pid); //signal from center left sensor
    error = left_pid_val - right_pid_val; // Positive position to right of line
    if (abs(error) <= ERROR_RANGE){
      left_servo.write(SERVO_L_FORWARD_MAX);
      right_servo.write(SERVO_R_FORWARD_MAX);
    }
    // Too right
    else if (error > ERROR_RANGE) {
      // Adjust left
      error_magnitude = abs(error)/(float)ERROR_RANGE;
      left_servo.write(SERVO_L_FORWARD_MAX - error_magnitude*SERVO_L_INCR_FORWARD);
      right_servo.write(SERVO_R_FORWARD_MAX + error_magnitude*SERVO_R_INCR_FORWARD);
    }
    else if (error < -ERROR_RANGE) {
      // Adjust right
      error_magnitude = abs(error)/(float)ERROR_RANGE;
      left_servo.write(SERVO_L_FORWARD_MAX + error_magnitude*SERVO_L_INCR_FORWARD);
      right_servo.write(SERVO_R_FORWARD_MAX - error_magnitude*SERVO_R_INCR_FORWARD);
    }
    delay(10);
  }
}
