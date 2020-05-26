#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ModbusTCPSlave.h>

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <ThingerESP8266.h>                                         //file header thinger.io

#define USERNAME "edijunaedi"                                   //masukan nama akun yg dibuat di thinger.io
#define DEVICE_ID "semarhom"                                  //masukan Device ID yg dibuat di thinger.io
#define DEVICE_CREDENTIAL "rKw9t&2EdG8k5b"                  //masukan nama DEVICE_CREDENTIAL yg tergenerate di thinger.io

#define SSID "barokah"                                       //masukan SSID/nama wifi yang digunakan  
#define SSID_PASSWORD "1hitam881"                     //masukan password wifi yang digunakan

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);       //objek thing

BlynkTimer timer1;

//modbus
byte ip[]={192, 168, 43, 99};
byte gateway[]={192, 168, 43, 1};
byte subnet[]={255, 255, 255, 0};
ModbusTCPSlave Mb;
unsigned long timer;
unsigned long checkRSSIMillis;
int sensorPin=A0;
int sensorValue=0;
int accelx, accely, accelz;

char auth[] = "TKsP1VIolN8WEDeYbFfGA_vjCBsV_Hd4";


const char* ssid = "barokah";
const char* password = "1hitam881";

const int led = 2;
unsigned long previousMillis =0;
const long interval = 1000;
int ledstate = LOW;

void setup() {
  
  thing.add_wifi(SSID, SSID_PASSWORD);                              //inisialisasi thing untuk koneksi ke wifi
  thing["led"] << digitalPin(LED_BUILTIN);                          //mengontrol LED yang terhubung di PIN D0 Nodemcu
  thing["millis"] >> outputValue(millis());//mambaca nilai waktu dari fungsi millis

  
  
  modbusSetup(); //modbus
  timer1.setInterval(1000L, myTimerEvent);
 
  pinMode(led, OUTPUT);
  
  Serial.begin(115200);
  //blynk
  Blynk.begin(auth, ssid, password,IPAddress(192,168,43,28),8080);
  
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  //blynkBlynk.run();
   timer1.run(); // Initiates BlynkTimer
thing.handle();
 // modbusLoop();

}

void modbusSetup(){
  Mb.begin(ssid, password, ip, gateway, subnet);
  delay(1);
  Mb.MBInputRegister[0]=100;
  Mb.MBInputRegister[1]=65500;
  Mb.MBInputRegister[2]=300;
  Mb.MBInputRegister[3]=400;
  Mb.MBInputRegister[4]=500;

  Mb.MBHoldingRegister[0]=1;
  Mb.MBHoldingRegister[1]=2;
  Mb.MBHoldingRegister[2]=3;
  Mb.MBHoldingRegister[3]=4;
  Mb.MBHoldingRegister[4]=5;

}


void modbusLoop(){
    sensorValue=analogRead(sensorPin);
  Mb.Run();
  delay(10);

  if (millis() - timer >=1000){
  timer = millis();
  Mb.MBInputRegister[1]++;
  }

  if (millis() - checkRSSIMillis >= 10000){
  checkRSSIMillis=millis();
  Mb.MBInputRegister[0]=checkRSSI();
  }
  Mb.MBHoldingRegister[4]=sensorValue;
}

byte checkRSSI(){
  byte quality;
  long rssi=WiFi.RSSI();
  if(rssi<=-100)
  quality=0;
  else if(rssi>=50)
   quality=100;
  else
  rssi=rssi+100;
  quality=byte(rssi*2);

  return quality;
}

BLYNK_WRITE(V1)
{
  //int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
 
    ledstate = not(ledstate);
    digitalWrite(led,ledstate);  
}

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  Mb.MBHoldingRegister[1]=pinValue;
}

BLYNK_WRITE(V3) {
  int x = param[0].asInt();
  int y = param[1].asInt();

  Mb.MBHoldingRegister[2]=x;
  Mb.MBHoldingRegister[3]=y;
}

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V5, millis() / 1000);
  Blynk.virtualWrite(V10,accelx );
  Blynk.virtualWrite(V11,accely );
  Blynk.virtualWrite(V12,accelz );
}

BLYNK_WRITE(V6) {
  int x = param[0].asInt();
  int y = param[1].asInt();
  int z = param[2].asInt();

  accelx=x;
  accely=y;
  accelz=z;
}
