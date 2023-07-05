#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include "time.h"
#include <string>
#include <esp_adc_cal.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


#include "addons/TokenHelper.h"

#include "addons/RTDBHelper.h"

boolean fall = false; 
boolean trigger1=false; 
boolean trigger2=false; 
boolean trigger3=false;
byte trigger1count=0; 
byte trigger2count=0;
byte trigger3count=0; 

// Insert your network credentials
#define WIFI_SSID "ADVAN START-6572BE"
#define WIFI_PASSWORD "96429DD7"

void send_event(const char *event);
const char *host = "maker.ifttt.com";
const char *privateKey = "dYfn40zTNG5-i-jmybct8M";

// Insert Firebase project API Key
#define API_KEY "AIzaSyCpS0YA5K-XoiO7ZSeo2P2tkIdESS2u39Y"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://data-ta-92c87-default-rtdb.asia-southeast1.firebasedatabase.app"
#define LED_PIN 4

// Define the ADC parameters
#define DEFAULT_VREF 3.3   // ADC reference voltage in mV
#define ADC_RESOLUTION 12   // ADC resolution in bits

// Pin assignment
const int voltageDividerPin = 34;  // GPIO pin connected to the voltage divider

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// user anon pake ini
bool signupOK = false;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String coor_x = "/x_coordinate"; 
String coor_y = "/y_coordinate";
String activityPath = "/activity";
String batteryPath ="/battery";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// MPU6050 sensor
Adafruit_MPU6050 mpu;  // I2C

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 15000;

// BLE
int scanTime = 5;  //In seconds
BLEScan* pBLEScan;
typedef struct {
  char address[17];  // 67:f1:d2:04:cd:5d
  int rssi;
  float dista;
} BeaconData;

struct data_BLE{
  float x;
  float y;
  float dist_ble;
};
data_BLE ble_Array[8];
struct IntersectionPoint{
  float x_co;
  float y_co;
};


int led_state = LOW;

uint8_t bufferIndex = 0;
BeaconData buffer[50];
float rssi_str;
float distance;

int compare(const void* a, const void* b) {
  data_BLE* s1 = (data_BLE*)a;
  data_BLE* s2 = (data_BLE*)b;
  return s1->dist_ble - s2->dist_ble;
}

IntersectionPoint getTrilateration(data_BLE position1, data_BLE position2, data_BLE position3) {
  float xa = position1.x;
  float ya = position1.y;
  float xb = position2.x;
  float yb = position2.y;
  float xc = position3.x;
  float yc = position3.y;
  float ra = position1.dist_ble;
  float rb = position2.dist_ble;
  float rc = position3.dist_ble;

  float S = (pow(xc, 2.) - pow(xb, 2.) + pow(yc, 2.) - pow(yb, 2.) + pow(rb, 2.) - pow(rc, 2.)) / 2.0;
  float T = (pow(xa, 2.) - pow(xb, 2.) + pow(ya, 2.) - pow(yb, 2.) + pow(rb, 2.) - pow(ra, 2.)) / 2.0;
  float intersectionY = abs(((T * (xb - xc)) - (S * (xb - xa))) / (((ya - yb) * (xb - xc)) - ((yc - yb) * (xb - xa))));
  float intersectionX = abs(((intersectionY * (ya - yb)) - T) / (xb - xa));

  IntersectionPoint result;
  result.x_co = intersectionX;
  result.y_co = intersectionY;
  return result;
}
//Battery Percentage
int calculateBatteryPercentage(float voltage) {
  float percentage = (voltage - 1.5) / (1.85 - 1.5) * 100.0;
  percentage = constrain(percentage, 0, 100);  // Ensure percentage is within the range 0-100
  return int(percentage);
}

// Initialize MPU6050
void initMPU() {
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

// BLE

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    extern uint8_t bufferIndex;
    extern BeaconData buffer[];
    if (bufferIndex >= 50) {
      return;
    }
    rssi_str = float(advertisedDevice.getRSSI());
    distance = pow(10, (-70 - rssi_str) / 42);

    if (advertisedDevice.haveRSSI()) {
      buffer[bufferIndex].rssi = advertisedDevice.getRSSI();
      buffer[bufferIndex].dista = distance;
    } else {
      buffer[bufferIndex].rssi = 0;
    }
    strcpy(buffer[bufferIndex].address, advertisedDevice.getAddress().toString().c_str());

    bufferIndex++;

    // Print everything via serial port for debugging
    //Serial.printf("MAC: %s \n", advertisedDevice.getAddress().toString().c_str());
    //Serial.printf("name: %s \n", advertisedDevice.getName().c_str());
    //Serial.printf("RSSI: %d \n", advertisedDevice.getRSSI());
    //Serial.printf("Distance: %f \n",distance);
  }
};

// Write float values to the database
void sendFloat(String path, float value) {
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)) {
    Serial.print("Writing value: ");
    Serial.print(value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_RESOLUTION);
  analogSetAttenuation(ADC_11db);  // Set the input attenuation to 11dB for a voltage range of 0-3.3V
  pinMode(LED_PIN,OUTPUT);
  // Initialize MPU6050 sensor
  initMPU();
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;


  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  // user anon
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  // Assign the maximum retry of token generation (user bukan anon)
  // config.max_token_generation_retry = 5;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  digitalWrite(LED_PIN,HIGH);
  float d1,d2,d3,d4,d5,d6,d7,d8;
  //BLE
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print("time: ");
    Serial.println(timestamp);

    parentPath = databasePath + "/" + "device1";
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float accX, accY, accZ, gyroX, gyroY, gyroZ, magAcc, magGyro;
    accX = a.acceleration.x - 0.3;
    accY = a.acceleration.y + 0.05;
    accZ = a.acceleration.z;
    gyroX = g.gyro.x * 57.3 + 2.4;
    gyroY = g.gyro.y * 57.3 + 0.8;
    gyroZ = g.gyro.z * 57.3 + 1;

    magAcc = pow(pow(accX, 2) + pow(accY, 2) + pow(accZ, 2), 0.5);
    magGyro = pow(pow(gyroX, 2) + pow(gyroY, 2) + pow(gyroZ, 2), 0.5);

    String activity;

    if (magAcc < 10 && magGyro < 3) {
      activity = "Tidur";
    } else if (magGyro > 50) {
      activity = "Jatuh";
    } else {
      activity = "Jalan";
    }

    if (magGyro > 50 && trigger1==false){ //if AM breaks lower threshold (0.4g)
   trigger1=true;
   fall=true;
   Serial.println("TRIGGER 1 ACTIVATED");
   }
 if (trigger1==true){
   trigger1count++;
   if (magGyro < 50){ //if AM breaks upper threshold (3g)
     trigger2=true;
     Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
     }
 }

 if (fall==true){ //in event of a fall detection
   Serial.println("FALL DETECTED");
   send_event("Fall Detection"); 
   fall=false;
   }

 if (trigger1count>=6){ //allow 0.5s for AM to break upper threshold
   trigger1=false; trigger1count=0;
   Serial.println("TRIGGER 1 DECACTIVATED");
   }
  delay(1000);

      for (uint8_t i = 0; i < bufferIndex; i++) {
    Serial.print(buffer[i].address);
    Serial.print(" : ");
    Serial.println(buffer[i].rssi);
    Serial.println(buffer[i].dista);
    if (buffer[i].address[0] == 'd' && buffer[i].address[1] == '3' && buffer[i].address[3] == 'b' && buffer[i].address[4] == 'b'){
      d1 = buffer[i].dista;
    }
    else if (buffer[i].address[0] == 'd' && buffer[i].address[1] == '0' && buffer[i].address[3] == '3' && buffer[i].address[4] == '7'){
      d2 = buffer[i].dista;
    }
    else if (buffer[i].address[0] == 'e' && buffer[i].address[1] == '1' && buffer[i].address[3] == '2' && buffer[i].address[4] == '4'){
      d3 = buffer[i].dista;
    }
    else if (buffer[i].address[0] == 'f' && buffer[i].address[1] == '0' && buffer[i].address[3] == '7' && buffer[i].address[4] == '3'){
      d4 = buffer[i].dista;
    }
    else if (buffer[i].address[0] == 'e' && buffer[i].address[1] == 'e' && buffer[i].address[3] == 'f' && buffer[i].address[4] == 'd'){
      d5 = buffer[i].dista;
    }
    else if (buffer[i].address[0] == 'e' && buffer[i].address[1] == 'e' && buffer[i].address[3] == 'a' && buffer[i].address[4] == 'a'){
      d6 = buffer[i].dista;
    }
    else if (buffer[i].address[0] == 'e' && buffer[i].address[1] == '1' && buffer[i].address[3] == 'a' && buffer[i].address[4] == '8'){
      d7 = buffer[i].dista;
    }
    else if (buffer[i].address[0] == 'f' && buffer[i].address[1] == '8' && buffer[i].address[3] == 'e' && buffer[i].address[4] == 'a'){
      d8 = buffer[i].dista;
    }
    // else if (buffer[i].address[0] == 'c' && buffer[i].address[1] == '4' && buffer[i].address[3] == '6' && buffer[i].address[4] == '3'){
    //   d9 = buffer[i].dista;
    // }
    // else if (buffer[i].address[0] == 'd' && buffer[i].address[1] == 'b' && buffer[i].address[3] == '3' && buffer[i].address[4] == 'a'){
    //   d10 = buffer[i].dista;
    // }
  }
 
  ble_Array[0].x= 4.12;
  ble_Array[0].y= 0.2;
  ble_Array[0].dist_ble=d1;
  ble_Array[1].x= 0.95;
  ble_Array[1].y=1.4;
  ble_Array[1].dist_ble=d2;
  ble_Array[2].x= 4.6;
  ble_Array[2].y=4;
  ble_Array[2].dist_ble=d3;
  ble_Array[3].x= 1.1;
  ble_Array[3].y=5.6;
  ble_Array[3].dist_ble=d4;
  ble_Array[4].x= 4.9;
  ble_Array[4].y= 6.5;
  ble_Array[4].dist_ble=d5;
  ble_Array[5].x= 0.1 ;
  ble_Array[5].y= 4.05;
  ble_Array[5].dist_ble=d6;
  ble_Array[6].x= 6.95;
  ble_Array[6].y= 1.9;
  ble_Array[6].dist_ble=d7;
  ble_Array[7].x= 7;
  ble_Array[7].y= 4.8;
  ble_Array[7].dist_ble=d8;
  // ble_Array[8].x= 4.28;
  // ble_Array[8].y= 8.2;
  // ble_Array[8].dist_ble=d9;
  // ble_Array[9].x= 8.4;
  // ble_Array[9].y= 2.16;
  // ble_Array[9].dist_ble=d10;

  qsort(ble_Array,8,sizeof(data_BLE),compare);
  IntersectionPoint trilateration = getTrilateration(ble_Array[0],ble_Array[1],ble_Array[2]);
  Serial.print(trilateration.x_co);
  Serial.print(trilateration.y_co);

  // Read the voltage from the voltage divider
  int rawValue = analogRead(voltageDividerPin);

  // Convert the ADC raw value to voltage
  float voltage = (rawValue * DEFAULT_VREF) / (1 << ADC_RESOLUTION);
  int batteryPercentage = calculateBatteryPercentage(voltage);
    
    // sendFloat(coor_x, trilateration.x_co);
    // sendFloat(coor_y, trilateration.y_co);
    json.set(coor_x.c_str(), String(trilateration.x_co));
    json.set(coor_y.c_str(), String(trilateration.y_co));
    json.set(activityPath.c_str(), activity);
    json.set(batteryPath.c_str(), String(batteryPercentage));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  // Stop BLE
  pBLEScan->stop();
  Serial.println("Scan done!");
  bufferIndex = 0;
  delay(2000);
}

void send_event(const char *event)
{
  Serial.print("Connecting to "); 
  Serial.println(host);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  while(client.connected())
  {
    if(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    } else {
      delay(50);
    };
  }
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}