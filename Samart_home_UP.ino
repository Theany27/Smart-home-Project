#include <Keypad.h> // Include keypad library
#include <Wire.h> // Include wire library for I2C
#include <LiquidCrystal_I2C.h> // Include LCD library for I2C
#include <Servo.h> // Include Servo library

// Pin definitions
//#define RED_LED_PIN A0
#define GREEN_LED_PIN A3
#define YELLOW_LED_PIN 11
#define white_LED_PIN A1
#define WHITE_LED_PIN 13
#define BUZZER_PIN A2
#define SERVO_PIN A0

#define IR_SENSOR_PIN 12 // Pin connected to the IR sensor

// Constants
#define PASS_LENGTH 4

// Keypad setup
const byte rows = 4;
const byte columns = 4;
char keyboard_keys[rows][columns] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte row_pins[rows] = {9, 8, 7, 6};
byte column_pins[columns] = {5, 4, 3, 2};

// Servo and password management
const int servo_open = 0;
const int servo_close = 90;
const unsigned long door_move = 12;
const unsigned long door_open = 2000;
const char password[PASS_LENGTH + 1] = "5555";
const char disable_alarm_password[PASS_LENGTH + 1] = "4444";
char entered_password[PASS_LENGTH + 1];
byte current_sign_position = 0;
int failed_attempts = 0;
bool alarm_triggered = false;

unsigned long prev_time = 0;
const long duration = 200;

// Keypad, Servo, and LCD instances
Keypad keypad = Keypad(makeKeymap(keyboard_keys), row_pins, column_pins, rows, columns);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function prototypes
void checkStatus();
void manageDoor();
void openDoor();
void closeDoor();
void disableAlarm();
void triggerAlarm();
void enterPassword();
void maskPassword(char key);
void checkIrSensor();

void setup() {
    Serial.begin(9600);
    servo.attach(SERVO_PIN);
    servo.write(servo_close);
    lcd.begin(16, 2);
    lcd.backlight();
//    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(WHITE_LED_PIN, OUTPUT);
    pinMode(white_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(IR_SENSOR_PIN, INPUT);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to");
    lcd.setCursor(1, 1);
    lcd.print("GROUP 4");
    delay(2000);
    enterPassword();
    keypad.setDebounceTime(20);
}

void loop() {
    checkStatus();
    checkIrSensor();
    if (alarm_triggered) {
        triggerAlarm();
    }
}

void enterPassword() {
    current_sign_position = 0;
    lcd.init();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password");
}

void maskPassword(char key) {
    if (current_sign_position < PASS_LENGTH) {
        entered_password[current_sign_position] = key;
        current_sign_position++;
        lcd.setCursor(current_sign_position, 1);
        lcd.print('*');
    }
}

void checkStatus() {
    char key = keypad.getKey();
    if (key) {
        if (key == '*') {
            entered_password[current_sign_position] = '\0';
            if (alarm_triggered) {
                if (strcmp(entered_password, disable_alarm_password) == 0) {
                    lcd.clear();
                    disableAlarm();
                    enterPassword();
                } else {
                    failed_attempts++;
                    lcd.clear();
                    lcd.print("Wrong password");
                    delay(1000);
                    enterPassword();
                }
            } else {
                if (strcmp(entered_password, password) == 0) {
                    lcd.clear();
                    lcd.print("Valid password");
                    failed_attempts = 0;
                    delay(1000);
                    manageDoor();
                    enterPassword();
                } else {
                    failed_attempts++;
                    lcd.clear();
                    lcd.print("Wrong password");
                    digitalWrite(YELLOW_LED_PIN, HIGH);
                    buzzer();
                    delay(2000);
                    if (failed_attempts >= 3) {
                        alarm_triggered = true;
                    }
                    digitalWrite(YELLOW_LED_PIN, LOW);
                    enterPassword();
                }
            }
        } else {
            maskPassword(key);
        }
    }
}

void openDoor() {
    lcd.clear();
    lcd.print("Opening the door");
    digitalWrite(white_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);

    for (int pos = servo_close; pos >= servo_open; pos--) {
        servo.write(pos);
        delay(door_move);
        tone(BUZZER_PIN, 1000);
    }
    digitalWrite(GREEN_LED_PIN, LOW);
    // digitalWrite(white_LED_PIN, LOW);
    noTone(BUZZER_PIN);
}

void closeDoor() {
    lcd.clear();
    lcd.print("Closing the door");
    digitalWrite(white_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    for (int pos = servo_open; pos <= servo_close; pos++) {
        servo.write(pos);
        delay(door_move);
        tone(BUZZER_PIN, 1000);
    }
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(white_LED_PIN, LOW);
    noTone(BUZZER_PIN);
    lcd.clear();
    lcd.print("Door closed");
    delay(1000);
}

void buzzer() {
    unsigned char i;
    for (i = 0; i < 200; i++) {
        tone(BUZZER_PIN, 2000);
        delay(1);
        noTone(BUZZER_PIN);
        delay(1);
    }
}

void triggerAlarm() {
     unsigned long current_time = millis();
    if (current_time - prev_time >= duration) {
        prev_time = current_time;
        //digitalWrite(RED_LED_PIN, HIGH);
        if (digitalRead(BUZZER_PIN) == LOW) {
            tone(BUZZER_PIN, 1000);
        } else {
            noTone(BUZZER_PIN);
        }
    }
}

void disableAlarm() {
    alarm_triggered = false;
    failed_attempts = 0;
    lcd.clear();
    lcd.print("Alarm Disabled");
    delay(1000);
//    digitalWrite(RED_LED_PIN, LOW);
    noTone(BUZZER_PIN);
}

void manageDoor() {
    openDoor();
    delay(door_open);
    closeDoor();
}

void checkIrSensor() {
    if (digitalRead(IR_SENSOR_PIN) == LOW) { // Assuming LOW indicates detection
        digitalWrite(WHITE_LED_PIN, HIGH);
    }else{
      digitalWrite(WHITE_LED_PIN, LOW);
    }
}
