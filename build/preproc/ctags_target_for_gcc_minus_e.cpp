# 1 "c:\\Users\\cgeor\\Documents\\PM_3D_MODELS\\src\\cnc_plotter.ino"
# 2 "c:\\Users\\cgeor\\Documents\\PM_3D_MODELS\\src\\cnc_plotter.ino" 2

// Stepper motors for X and Y axis






// Directins





// Servo motor for Z axis

Servo myservo;

// Pen states


bool pen_state = true;

// Global position variables
int x_pos = 0;
int y_pos = 0;
// Execute a single step of the stepper motor
void step(int dir, int axis)
{
 int translated_dir = 0;
 if (dir == 1) {
  translated_dir = 0x0;
 } else {
  translated_dir = 0x1;
 }

 digitalWrite(axis, 0x1);
 digitalWrite(axis + 3, translated_dir);
 delayMicroseconds(50);
 digitalWrite(axis, 0x0);
}

void pen_up()
{
 if (pen_state == false) {
  myservo.write(90);
  pen_state = true;
 }
}

void pen_down()
{
 if (pen_state == true) {
  myservo.write(0);
  pen_state = false;
 }
}




// Move stepper motors by StepsX and StepsY steps
int move(int stepsX, int stepsY, bool new_pen_state)
{
 int counter = ((((stepsX)>0?(stepsX):-(stepsX)))>(((stepsY)>0?(stepsY):-(stepsY)))?(((stepsX)>0?(stepsX):-(stepsX))):(((stepsY)>0?(stepsY):-(stepsY))));

 int dirX = stepsX >= 0 ? 1 : -1;
 int dirY = stepsY >= 0 ? 1 : -1;

 // Safeguards to prevent the stepper motors from going out of bounds
 if (x_pos + stepsX < 0 || x_pos + stepsX > 700) {
  Serial.println("X out of bounds" + String(x_pos + stepsX));
  return -1;
 }

 if (y_pos + stepsY < 0 || y_pos + stepsY > 1000) {
  Serial.println("Y out of bounds" + String(y_pos + stepsY));
  return -1;
 }

 if (new_pen_state != pen_state) {
  if (new_pen_state == true) {
   pen_up();
  } else {
   pen_down();
  }
  delay(50);
 }

 pen_state = new_pen_state;
 for (int i = 0; i < counter; i++) {
        if (i < ((stepsX)>0?(stepsX):-(stepsX))) {

   step(dirX, 2);

   if (dirX == 1) {
    x_pos++;
   } else {
    x_pos--;
   }
  }
  if (i < ((stepsY)>0?(stepsY):-(stepsY))) {
   step(dirY, 3);

   if (dirY == 1) {
    y_pos++;
   } else {
    y_pos--;
   }
  }

  delayMicroseconds(2500);
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
 if (delta_x > delta_y) {
  if (delta_y == 0) {
   ratio = 1;
  } else {
   ratio = ((delta_x / delta_y)>=0?(long)((delta_x / delta_y)+0.5):(long)((delta_x / delta_y)-0.5));
   stepsY = 1 * (delta_y < 0 ? -1 : 1);
  }

  stepsX = ratio * (delta_x < 0 ? -1 : 1);
 } else {
  if (delta_x == 0) {
   ratio = 1;
  } else {
   ratio = ((delta_y / delta_x)>=0?(long)((delta_y / delta_x)+0.5):(long)((delta_y / delta_x)-0.5));
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

 for (int i = 0; i < ((((delta_x)>0?(delta_x):-(delta_x)))>(((delta_y)>0?(delta_y):-(delta_y)))?(((delta_x)>0?(delta_x):-(delta_x))):(((delta_y)>0?(delta_y):-(delta_y)))); i++) {

  int rc = move(stepsX, stepsY, pen_state);
  if (rc < 0)
   break;
 }
}

void setup()
{
 // X Stepper Motor
 pinMode(2, 0x1);
 pinMode(5, 0x1);
 // Y Stepper Motor
 pinMode(3, 0x1);
 pinMode(6, 0x1);
 myservo.attach(A3);
 Serial.begin(9600);
}

// Moves
void random_moves()
{
 int delta_x = 0;
 int delta_y = 0;
 int dirX = 0;
 int dirY = 0;

 delta_x = random(-x_pos, 700 - x_pos);
 delta_y = random(-y_pos, 1000 - y_pos);

 move(delta_x, delta_y, false);
}

void loop()
{
 Serial.println("Starting sequence");
 while (Serial.available() == 0)
  ;

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
  move_to(300, 0, false);
        move_to(300, 300, false);
        move_to(0, 300, false);
  move_to(0, 0, false);
  // delay(2000);
  // move_to(0, 100, PEN_DOWN);
  // delay(2000);
  // move_to(-100, 0, PEN_DOWN);
  // delay(2000);
  // move_to(0, -100, PEN_DOWN);
  // delay(2000);
  // move_to(0, 0, PEN_UP);
 }
}
