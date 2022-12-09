#include <bluefruit.h>

BLEUuid           VARS_UUID_SERV("00000100-1212-EFDE-1523-785FEABCD123");
BLEService        varsServ(VARS_UUID_SERV);
BLEUuid           ANGLE_UUID_CHAR("00000101-1212-EFDE-1523-785FEABCD123");
BLECharacteristic angleChar(ANGLE_UUID_CHAR);
BLEUuid           VELOCITY_UUID_CHAR("00000102-1212-EFDE-1523-785FEABCD123");
BLECharacteristic velocityChar(VELOCITY_UUID_CHAR);

uint32_t analogMax = 1023;
uint32_t analogRef = analogMax / 2;

const uint16_t max_len = 20;
char buf[max_len];

double angle, velocity, prevAngle;
uint32_t Told, Tnew;

// ==========================================
// STARTUP BLOCK
// ==========================================
void setupChars() {
  varsServ.begin();
  
  angleChar.setProperties(CHR_PROPS_NOTIFY);
  angleChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  angleChar.setMaxLen(max_len);
  angleChar.begin();

  velocityChar.setProperties(CHR_PROPS_NOTIFY);
  velocityChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  velocityChar.setMaxLen(max_len);
  velocityChar.begin();
}

void setupBluetooth() {
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  Bluefruit.configUuid128Count(15);
  
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  setupChars(); 

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(varsServ);

  Bluefruit.ScanResponse.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void setup() {
  Serial.begin(9600);
  setupBluetooth();
  angle = 0;
  Tnew = 0;
}

// ==========================================
// RUNNING BLOCK
// ==========================================
void loop() {
  prevAngle = angle;
  Told = Tnew;
  Tnew = millis();
  uint32_t H1 = analogRead(A0);
  uint32_t H2 = analogRead(A1);
  angle = getAngle(H1, H2);
  velocity = getVelocity (Told, Tnew, prevAngle, angle);

  memset(buf,0,strlen(buf));
  sprintf(buf, "%.2f", angle);
  angleChar.notify(buf, strlen(buf));

  memset(buf,0,strlen(buf));
  sprintf(buf, "%.2f", velocity);
  velocityChar.notify(buf, strlen(buf));

  Serial.print(Tnew);
  Serial.print(",");
  Serial.print(H1);
  Serial.print(",");
  Serial.print(H2);
  Serial.print(",");
  Serial.print(angle);
  Serial.print(",");
  Serial.println(velocity);
}

double getAngle(uint32_t H1, uint32_t H2) {
  double analogCal, result;
  analogCal = double(H1)-analogRef;
  analogCal /= analogRef;
  if(H2 < analogRef) {
    result = asin(analogCal);
    result += PI*1.5;
    result *= RAD_TO_DEG;
  } else {
    result = -asin(analogCal);
    result += PI*0.5;
    result *= RAD_TO_DEG;
  }
  return result;
}

double getVelocity(uint32_t Told, uint32_t Tnew, double prevAngle, double angle){
  double deltaT = Tnew - Told;
  double deltaA = angle - prevAngle;
  return deltaA/deltaT;
}


// ==========================================
// HELPER FUNCTIONS
// ==========================================
void connect_callback(uint16_t conn_handle)
{
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}


void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}