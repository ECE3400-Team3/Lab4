#include <Servo.h>

int right_pid = A0; // right center
int left_pid = A1; // left center
int right_turn = 8; // right turn sensor 
int left_turn = 9; // left turn sensor
int right_pid_val = 0; 
int left_pid_val = 0;
int right_turn_val = 0;
int left_turn_val = 0;

//PID terms
float kp = 0.005;
float ki = 0.0;
float kd = 0;
float set_point = 0;
float curr_pos = 0;
float prev_error = 0;
float error = 0;
float integral = 0;
float derivative = 0;
float u;
float dt = 10;
int printtime = 0;

// Servo objects
Servo left_servo;
Servo right_servo;

#define LEFT_MAX_SPEED   95
#define RIGHT_MAX_SPEED  85

int left_speed = 92; //left forward max is 180
int right_speed = 88; //right forward max is 0


void setup() {
  Serial.begin(9600);
  right_servo.attach(11); 
  left_servo.attach(10);
  pinMode(right_turn, INPUT);
  pinMode(left_turn, INPUT);
  //start_intersection();
}

void loop() {


  // Read analog values from two center sensors
  right_pid_val = analogRead(right_pid); //signal from line sensor(s) on right side
  left_pid_val = analogRead(left_pid); //signal from line sensor(s) on left side

  // Read digital values from left and right turn sensors (Schmitt Triggers)
  right_turn_val = digitalRead(right_turn);
  left_turn_val = digitalRead(left_turn);

  if(right_turn_val&&left_turn_val){
    //turn function
    turn_right_at_intersection();
  }
  if (right_turn_val){
    //correct toward right
  }
  if (left_turn_val){
    //correct toward left
  }

  
  //PID control
  curr_pos = right_pid_val-left_pid_val;
  error = set_point - curr_pos; //error positive when right of line
  integral = integral + error*dt;
  derivative = (error-prev_error)/dt;
  u = kp*error + ki*integral + kd*derivative;
  prev_error = error;

  left_speed = left_speed - (u); //left servo slow down when u is pos, speed up when u is neg
  right_speed = right_speed - (u);

  drive(left_speed, right_speed);

  // print once per second
  if(printtime >= 1000){
    printtime = 0;
    Serial.println(right_pid_val);
    Serial.println(left_pid_val);
    Serial.println();
  }
  printtime = printtime + dt;
  delay(dt);
}

void drive(int left_s, int right_s){
 if (left_s > LEFT_MAX_SPEED){
  left_s = LEFT_MAX_SPEED;
 }
 else if(left_s <= 90){
  left_s = 91;
 }
 if (right_s < RIGHT_MAX_SPEED){
  right_s = RIGHT_MAX_SPEED;
 }
 else if(right_s >= 90){
  right_s = 89;
 }

 left_servo.write(left_s);
 right_servo.write(right_s);

 left_speed = left_s;
 right_speed = right_s;
}

void start_intersection(){
  right_turn_val = digitalRead(right_turn);
  left_turn_val = digitalRead(left_turn);
    
  // Read digital values from left and right turn sensors (Schmitt Triggers)
  while(right_turn_val||left_turn_val){
    right_turn_val = digitalRead(right_turn);
    left_turn_val = digitalRead(left_turn);
    if(right_turn_val==0){
      right_speed = 90;      left_speed = 91;
    }
    if(left_turn_val==0){
      right_speed = 89;
      left_speed = 90;
    }
    if(right_turn_val&&left_turn_val){
      if(right_speed == 89){
        right_speed = 91;
        left_speed = 91;
      }
      else {
        right_speed = 89;
        left_speed = 89;
      }
    }
    drive(left_speed, right_speed);
    delay(10);
  }
}


void turn_left_at_intersection(){
  left_speed = 85;
  right_speed = 85;
  left_servo.write(left_speed);
  right_servo.write(right_speed);
  while(left_turn_val){
    // Read digital values from left and right turn sensors (Schmitt Triggers)
    left_turn_val = digitalRead(left_turn);
  }
}
  
void turn_right_at_intersection(){
  left_speed = 95;
  right_speed = 95;
  left_servo.write(left_speed);
  right_servo.write(right_speed);
  while(right_turn_val){
    // Read digital values from left and right turn sensors (Schmitt Triggers)
    right_turn_val = digitalRead(right_turn);
  }
}

