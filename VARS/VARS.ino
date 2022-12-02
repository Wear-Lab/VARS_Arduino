double analogMax = 1023;
double analogRef = analogMax / 2;
double analogCal = 0;
double angle = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print(analogRead(A0));
  Serial.print(",");
  Serial.print(analogRead(A1));
  Serial.print(",");
  // Calculate Angle
  analogCal = double(analogRead(A0))-analogRef;
  analogCal /= analogRef;
  if(analogRead(A1) < analogRef) {
    angle = asin(analogCal);
    angle += PI*1.5;
    angle *= RAD_TO_DEG;
    Serial.print(angle);
  } else {
    angle = -asin(analogCal);
    angle += PI*0.5;
    angle *= RAD_TO_DEG;
    Serial.print(angle);
  }
  Serial.println(" deg");
}
