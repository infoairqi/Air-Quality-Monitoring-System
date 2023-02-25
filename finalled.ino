#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Wire.h>
#include "SSD1306.h"
#include <MQ7.h>
#include <MQ135.h>
#include <SoftwareSerial.h>


#define DHTPIN            D5
#define S0                D0
#define S1                D1
#define analogpin         A0

SoftwareSerial pmsSerial(D6, D7);
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);  // I2C / TWI

float h;
float t;

String f;

float gas;
float CO;
float pm1;
float pm2_5;
float pm10;
float ppm;
float ppm1;

float hi;
float lo;
float bphi;
float bplo;
float c;
float aqif;

float m = -0.3376; //Slope 
float b = 0.7165; //Y-Intercept 
float R0 = 21.91; //Sensor Resistance in fresh air from previous code

float m1 = -0.6527; //Slope 
float b1 = 1.30; //Y-Intercept
float R01 = 3.12; //Sensor Resistance 


//void draw(void);
//void draw2(void);

void setup() {
  Serial.begin(9600);
  
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(A0, INPUT);
  /*
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
   u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
   u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }*/
  pmsSerial.begin(9600);
  
  dht.begin();

}

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data;

void loop() {
  // Wait a few seconds between measurements.
    gas = analogRead(analogpin);
    CO = analogRead(analogpin);

    delay(3000);
    //gas = analogRead(A2);
    //CO = analogRead(A3);
    
    float sensor_volt; //Define variable for sensor voltage 
    float RS_gas; //Define variable for sensor resistance

    // Address 00
    digitalWrite(S0, LOW);
    digitalWrite(S1, LOW);
  
    float ratio; //Define variable for ratio
    float sensorValue = gas; //Read analog values of sensor  
    sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage 
    RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas
    ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air
    double ppm_log = (log10(ratio)-b)/m; //Get ppm value in linear scale according to the the ratio value  
    ppm = pow(10, ppm_log); //Convert ppm value to log scale 
    Serial.print("\nGAS PPM = ");
    Serial.println(ppm);

    // Address 10
    digitalWrite(S0, HIGH);
    digitalWrite(S1, LOW);
  
    float sensor_volt1; //Define variable for sensor voltage 
    float RS_gas1; //Define variable for sensor resistance  
    float ratio1; //Define variable for ratio
    float sensorValue1 = CO; //Read analog values of sensor   
    sensor_volt1 = sensorValue1*(5.0/1023.0); //Convert analog values to voltage 
    RS_gas1 = ((5.0*10.0)/sensor_volt1)-10.0; //Get value of RS in a gas
    ratio1 = RS_gas1/R01;  // Get ratio RS_gas/RS_air
    double ppm_log1 = (log10(ratio1)-b1)/m1; //Get ppm value in linear scale according to the the ratio value  
    double ppm1 = pow(10, ppm_log1); //Convert ppm value to log scale 
    Serial.print("CO PPM = ");
    Serial.println(ppm1);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius
  t = dht.readTemperature();
  // Read temperature as Fahrenheit
  Serial.println("Humidity: ");Serial.print(h);
  Serial.println("Temperature: ");Serial.print(t);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;

  }
  delay(2000);
  if (readPMSdata(&pmsSerial))
  {
  // reading data was successful!
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_env);
    Serial.println("---------------------------------------");
    //Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(data.particles_03um);
    //Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(data.particles_25um);
  // Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(data.particles_50um);
    Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");
  }

// Converting the sensor readings to string values that the LCD can display
String pm1String = String(data.pm10_standard);
String pm25String = String(data.pm25_standard);
String pm100String = String(data.pm100_standard);

  if(ppm>0 && ppm <50)
    {
       Serial.println("\nAQI : ");
       Serial.print(ppm);
       f = "Good";
       Serial.println("\nAir Quality : Good");
    }
    else if(ppm >51 && ppm<150)
    {
       Serial.println("\nAQI : ");
       Serial.print(ppm);
       f = "Moderate";
       Serial.println("\nAir Quality : Moderate");
    }
    else if(ppm >151 && ppm<250)
    {
       Serial.println("\nAQI : ");
       Serial.print(ppm);
       f = "High";
       Serial.println("\nAir Quality : High");
    }
    else if(ppm >251 )
    {
       Serial.println("\nAQI : ");
       Serial.print(ppm);
       f = "Extreme";
       Serial.println("\nAir Quality : Extreme");
    }


 /*{ u8g.firstPage();
  do {
    draw();
  } while (u8g.nextPage() );
  delay(3500);
  u8g.firstPage();
  do {
    draw2();
  } while (u8g.nextPage());
  delay(1000);
 }
*/
  // Compute heat index
  // Must send in temp in Fahrenheit!

  

}

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
 
  /* debugging
  for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
  }
  Serial.println();
  */
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);
 
/*  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }*/
  // success!
  return true;
}

/*void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(0, 8);
  u8g.print(" Humidity is ");
  u8g.print(h); u8g.print(" %");
  u8g.setPrintPos(0, 25);
  u8g.print(" Temperature is ");
  u8g.print(t); u8g.print(" %");

}

void draw2(void) {
  u8g.setFont(u8g_font_profont12);
  u8g.setPrintPos(0, 10);
  u8g.print(" AQI: ");
  u8g.print(ppm);u8g.print("ppm"); 
  u8g.setPrintPos(0, 25);
  u8g.print(" Quality : ");
  u8g.print(f);
}*/
