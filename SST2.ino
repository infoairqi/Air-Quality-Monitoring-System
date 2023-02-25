#include <MQ7.h>
#include <MQ135.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#define DHTTYPE DHT22 // type of the temperature sensor
#define DHTPIN D5 //--> The pin used for the DHT11 sensor is Pin D1 = GPIO5
DHT dht(DHTPIN, DHTTYPE); //--> Initialize DHT sensor, DHT dht(Pin_used, Type_of_DHT_Sensor);

#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

const char* ssid = "AAP"; //--> Your wifi name or SSID.
const char* password = "Chutiye@9"; //--> Your wifi password.


//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbxCy4dqnkEcN76r29s0gUWb7Mcj6szbaJ48c5z7CBVfHwDrNaoeshTZb7VcZssCqSy0"; //--> spreadsheet script ID

float CO;
float gas;

float m1 = -0.6527; //Slope 
float b1 = 1.30; //Y-Intercept
float R01 = 3.12; //Sensor Resistance 

float m = -0.3376; //Slope 
float b = 0.7165; //Y-Intercept 
float R0 = 21.91; //Sensor Resistance in fresh air from previous code

float c;
float g;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(500);

  dht.begin();  //--> Start reading DHT11 sensors
  delay(500);
  
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");

  pinMode(A0, INPUT);
    
  pinMode(ON_Board_LED,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  delay(500);
  Serial.print(h);
  Serial.print(t);
  
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor !");
    delay(500);
    return;
  }
  String Temp = "Temperature : " + String(t) + " °C";
  String Humi = "Humidity : " + String(h) + " %";
  Serial.println(Temp);
  Serial.println(Humi);

  CO = analogRead(A0);
    float sensor_volt1; //Define variable for sensor voltage 
    float RS_gas1; //Define variable for sensor resistance  
    float ratio1; //Define variable for ratio
    float sensorValue1 = CO; //Read analog values of sensor   
    sensor_volt1 = sensorValue1*(5.0/1023.0); //Convert analog values to voltage 
    RS_gas1 = ((5.0*10.0)/sensor_volt1)-10.0; //Get value of RS in a gas
    ratio1 = RS_gas1/R01;  // Get ratio RS_gas/RS_air
    double ppm_log1 = (log10(ratio1)-b1)/m1; //Get ppm value in linear scale according to the the ratio value  
    double ppm1 = pow(10, ppm_log1); //Convert ppm value to log scale 
    c = ppm1;
    Serial.println(c);

/*    gas = analogRead(A2);
    float sensor_volt; //Define variable for sensor voltage 
    float RS_gas; //Define variable for sensor resistance
     float ratio; //Define variable for ratio
    float sensorValue = gas; //Read analog values of sensor  
    sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage 
    RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas
    ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air
    double ppm_log = (log10(ratio)-b)/m; //Get ppm value in linear scale according to the the ratio value  
    ppm = pow(10, ppm_log); //Convert ppm value to log scale 
    g = ppm;
    Serial.print("\nGAS PPM = ");
    Serial.println(ppm);*/
    
  sendData(t, h,c); //--> Calls the sendData Subroutine,g
  delay(1000);
  
}

// Subroutine for sending data to Google Sheets
//, float cm    , float ga
void sendData(float tem, int hum, float cm) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String string_cm = String(cm);
//  String string_gas = String(ga);
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity="+ string_humidity + "&CO=" + string_cm ;
  //+ string_humidity + "&CO=" + string_cm    + "&GAS=" + string_gas 
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
} 
