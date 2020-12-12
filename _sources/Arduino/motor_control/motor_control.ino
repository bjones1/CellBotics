// ****************************************
// |docname| - a test motor control program
// ****************************************
// This project demonstrates basic control of the DC motors.
//
// .. figure:: schematic.png
//  :scale: 25%
//
//  This is the schematic for the robot.
//
//
// Macros
// ======
// The pushbutton on the evaluation board is connected to GPIO0.
#define PB1 (0)
// The LED on the evaluation board is connected to GPIO2.
#define LED1 (2)

// Motor connection for the robot.
#define PWM_A (23)
#define MOTOR_A2 (22)
#define MOTOR_A1 (1)


// Functions
// =========
void setup() {
    pinMode(PB1, INPUT);
    pinMode(LED1, OUTPUT);

    pinMode(PWM_A, OUTPUT);
    pinMode(MOTOR_A2, OUTPUT);
    pinMode(MOTOR_A1, OUTPUT);
}

void loop() {
    // Read a value from the pushbutton, then write it to the LED.
    digitalWrite(LED1, digitalRead(PB1));

    digitalWrite(PWM_A, 1);
    digitalWrite(MOTOR_A2, 0);
    digitalWrite(MOTOR_A1, 1);
}
