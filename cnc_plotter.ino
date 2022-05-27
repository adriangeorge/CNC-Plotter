#include "Servo.h"

// Stepper motors for X and Y axis
#define X_STEP_PIN 2
#define X_DIR_PIN 5
#define Y_STEP_PIN 3
#define Y_DIR_PIN 6
#define ENABLE_PIN 8

// Directins
#define FORWARDS 1
#define BACKWARDS -1

#define PULSE_DELAY 50
#define STEP_DELAY 2500
// Servo motor for Z axis
#define SERVO_PIN A3
Servo myservo;

// Pen states
#define PEN_UP true
#define PEN_DOWN false
bool pen_state = PEN_DOWN;

// Global position variables

int x_pos = 0;
int y_pos = 0;
// Execute a single step of the stepper motor
void step(int dir, int axis)
{
	int translated_dir = 0;
	if (dir == FORWARDS) {
		translated_dir = LOW;
	} else {
		translated_dir = HIGH;
	}

	digitalWrite(axis, HIGH);
	digitalWrite(axis + 3, translated_dir);
	delayMicroseconds(PULSE_DELAY);
	digitalWrite(axis, LOW);
}

void pen_up()
{
	if (pen_state == PEN_DOWN) {
		pen_state = PEN_UP;
		myservo.write(0);
	}
}

void pen_down()
{
	if (pen_state == PEN_UP) {
		myservo.write(90);
		pen_state = PEN_DOWN;
	}
}

#define MAX_Y 1000
#define MAX_X 700

// Move stepper motors by StepsX and StepsY steps
int move(int stepsX, int stepsY, bool new_pen_state)
{
	int counter = max(abs(stepsX), abs(stepsY));

	int dirX = stepsX >= 0 ? FORWARDS : BACKWARDS;
	int dirY = stepsY >= 0 ? FORWARDS : BACKWARDS;

	// Safeguards to prevent the stepper motors from going out of bounds
	if (x_pos + stepsX < 0 || x_pos + stepsX > MAX_X) {
		Serial.println("X out of bounds" + String(x_pos + stepsX));
		return -1;
	}

	if (y_pos + stepsY < 0 || y_pos + stepsY > MAX_Y) {
		Serial.println("Y out of bounds" + String(y_pos + stepsY));
		return -1;
	}

	if (new_pen_state != pen_state) {
		if (new_pen_state == PEN_UP) {
			pen_up();
		} else {
			pen_down();
		}
		delay(50);
	}

	pen_state = new_pen_state;
	for (int i = 0; i < counter; i++) {
		if (i < abs(stepsX)) {

			step(dirX, X_STEP_PIN);

			if (dirX == 1) {
				x_pos++;
			} else {
				x_pos--;
			}
		}
		if (i < abs(stepsY)) {
			step(dirY, Y_STEP_PIN);

			if (dirY == 1) {
				y_pos++;
			} else {
				y_pos--;
			}
		}

		delayMicroseconds(STEP_DELAY);
	}
	return 1;
}

void move_to(int x, int y, bool pen_state)
{
	Serial.println("Current position: " + String(x_pos) + " " +
		       String(y_pos));
	float delta_x = x - x_pos;
	float delta_y = y - y_pos;
	int ratio = 0;

	int stepsX = 0;
	int stepsY = 0;
	if (abs(delta_x) > abs(delta_y)) {
		if (delta_y == 0) {
			ratio = 1;
		} else {
			ratio = round(delta_x / delta_y);
			stepsY = 1 * (delta_y < 0 ? -1 : 1);
		}

		stepsX = ratio * (delta_x < 0 ? -1 : 1);
	} else {
		if (delta_x == 0) {
			ratio = 1;
		} else {
			ratio = round(delta_y / delta_x);
			stepsX = 1 * (delta_x < 0 ? -1 : 1);
		}

		stepsY = ratio * (delta_y < 0 ? -1 : 1);
	}

	Serial.println("------------------------------------");
	Serial.println("Moving to: X:" + String(x) + " Y:" + String(y));
	Serial.println("Steps: " + String(stepsX) + " " + String(stepsY));
	Serial.println("Pen state: " + String(pen_state));
	Serial.println("Delta X: " + String(delta_x) +
		       " Delta Y: " + String(delta_y));
	Serial.println("Ratio: " + String(ratio));
	Serial.println("------------------------------------");

	for (int i = 0; i < max(abs(delta_x), abs(delta_y)); i++) {

		int rc = move(stepsX, stepsY, pen_state);
		if (rc < 0)
			break;
	}
}

void setup()
{
	// X Stepper Motor
	pinMode(X_STEP_PIN, OUTPUT);
	pinMode(X_DIR_PIN, OUTPUT);
	// Y Stepper Motor
	pinMode(Y_STEP_PIN, OUTPUT);
	pinMode(Y_DIR_PIN, OUTPUT);
	myservo.attach(SERVO_PIN);
	Serial.begin(9600);
}

// Moves
void random_moves()
{
	int delta_x = 0;
	int delta_y = 0;
	int dirX = 0;
	int dirY = 0;

	delta_x = random(-x_pos, MAX_X - x_pos);
	delta_y = random(-y_pos, MAX_Y - y_pos);

	move(delta_x, delta_y, PEN_DOWN);
}

void loop()
{
	Serial.println("Starting sequence");
	while (Serial.available() == 0) {
		move_to(0, 300, PEN_UP);
		delay(500);
		move_to(300,300, PEN_DOWN);
		delay(500);
		move_to(300, 0, PEN_DOWN);
		delay(500);
		move_to(0, 0, PEN_DOWN);
		delay(500);
	}

	String str = Serial.readString();
	Serial.println("Received: [" + str + "\b]" + " current position " +
		       String(x_pos) + "," + String(y_pos));

	if (str.startsWith("G01")) {

		Serial.println(
		    str.substring(str.indexOf("X"), str.indexOf("Y")));
		Serial.println(
		    str.substring(str.indexOf("Y"), str.indexOf("Z")));
		int x = str.substring(str.indexOf("X") + 1, str.indexOf("Y"))
			    .toInt();
		int y = str.substring(str.indexOf("Y") + 1, str.indexOf("Z"))
			    .toInt();

		int pen_state =
		    str.substring(str.indexOf("Z") + 1, str.length()).toInt();

		move_to(x, y, pen_state);
	} else {
		move_to(0, 100, PEN_DOWN);
		delay(1000);
		move_to(100, 0, PEN_DOWN);
		delay(1000);
		move_to(0, 100, PEN_DOWN);
		delay(1000);
		move_to(0, 0, PEN_UP);
	}
}