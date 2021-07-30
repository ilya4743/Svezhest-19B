#include <SoftwareSerial.h>
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>   
#include <ESP8266WebServer.h>   
#include <ESP8266HTTPUpdateServer.h>  
#include <FS.h>            
#include <ArduinoJson.h>   
#include "Adafruit_CCS811.h"
#include "Adafruit_BME280.h"
#include <LiquidCrystal_I2C.h>      

#define CO2_RX_MHZ19 D3                 //Порт RX MH-Z19B
#define CO2_TX_MHZ19 D4                 //Порт RX MH-Z19B
#define CCS811_ADDR 0x5B                //Адрес CCS811
#define BME280_ADDR 0x76                //Адрес BME280
#define VER "2021-06-20"                //Дата последней прошивки
#define Akey "Vozduh01"                 // Ключ доступа к БД на сервере

ESP8266HTTPUpdateServer httpUpdater;
HTTPClient http;
ESP8266WebServer server(80); 
Adafruit_CCS811 CCS811;
Adafruit_BME280 BME280;
SoftwareSerial SerialCO2_MHZ19(CO2_TX_MHZ19, CO2_RX_MHZ19); // RX, TX
LiquidCrystal_I2C lcd(0x27,20,4);   
String LCDmode = "1";        //Режим отображения по умолчанию IP
String _ssid     = "rt"; // SSID сети
String _password = "apsI0gqvnFn";  // Пароль сети
String _ssidAP = "TestVosduha";     // SSID точки доступа
String _passwordAP = "";     // Пароль точки доступа
String jsonConfig = "{}";    //Переменная, хранящая файл конфигурации
String jsonStatus = "{}";    //Переменная, хранящая файл статуса (толком не используется)

unsigned int MHZ19_CO2=0, CCS811_eCO2=0, CCS811_TVOC=0;         //хранение показаний
float BME280_humidity=0, BME280_celsTemp=0, BME280_pressure=0;  //хранение показаний

bool net=false;       //удалось ли подключение
bool _BME280=false;   //работает ли датчик BME280
bool _CCS811=false;   //работает ли CCS811
bool MHZ19B=false;    //работает ли MH-Z19B
IPAddress myIP;

void setup(void) {
  lcd.begin();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  //Запуск файловой системы
  FSInit();
  lcd.print("Loading Сonfig");
  //Загрузка конфигурации
  LoadConfig();
  delay(3000);
  lcd.setCursor(0, 2);
  lcd.print("Connect to");
  lcd.setCursor(0,3);
  lcd.print(_ssid);
  // Подключение к сети
  WIFIinit();
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Init sensors");
  //Инициализация датчиков
  SensorsInit();  
  //Запуск Web-сервера
  lcd.setCursor(0, 1);
  delay(2000);
  lcd.print("Start Web Server");
  WebServerInit();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 2);
  lcd.print("OK");
  delay(2000);
  if(net)
    myIP=WiFi.localIP();
  else
    myIP = WiFi.softAPIP();
}

unsigned long timing=0;
unsigned int t1=0;
unsigned long state=0;
unsigned long state1=0;

void loop(void) {
  if(millis()-timing>=5000)
  {
    timing=millis();
    getSensors();
    t1++;
    if(t1==12)
    {
      t1=0;
      CheckWifi();
    }
    switch(LCDmode[0])
    {
      case '0':
      {
        Serial.print(" pokazania ");Serial.println(state);
        switch(state)
        {
          case 0:
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("BME280");
            lcd.setCursor(0,1);
            lcd.print("T: "); lcd.print(BME280_celsTemp,1); lcd.print((char)223); lcd.print("C ");
            lcd.setCursor(0,2);
            lcd.print("H: "); lcd.print(BME280_humidity,1); lcd.print(" %");
            lcd.setCursor(0,3);
            lcd.print("P: "); lcd.print(BME280_pressure,1); lcd.print(" mmHg");
            lcd.setCursor(16,3);
            lcd.print(WiFi.RSSI());
            break;
          }
  
          case 1:
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("MH-Z19B");
            lcd.setCursor(0,1);
            lcd.print("CO2: "); lcd.print(MHZ19_CO2,1); lcd.print(" ppm");
            lcd.setCursor(16,3);
            lcd.print(WiFi.RSSI());
            break;
          }
  
          case 2:
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("CCS811");
            lcd.setCursor(0,1);
            lcd.print("eCO2: "); lcd.print(CCS811_eCO2,1); lcd.print(" ppm");
            lcd.setCursor(0,2);
            lcd.print("TVOC: "); lcd.print(CCS811_TVOC,1); lcd.print(" ppb");
            lcd.setCursor(16,3);
            lcd.print(WiFi.RSSI());
            break;
          }
        }
        server.send(200, "text/plain", "Показания");
        break;
      }
      
      case '1':
      {
        Serial.println("IP ");
        if(net)
        {
          lcd.clear();      
          lcd.setCursor(0,0);
          lcd.print(_ssid);
          lcd.setCursor(0,1);
          lcd.print(myIP); 
          lcd.setCursor(16,3);
          lcd.print(WiFi.RSSI());
        }
        else
        {
          lcd.clear();      
          lcd.setCursor(0,0);
          lcd.print("AP:");lcd.print(_ssidAP);
          lcd.setCursor(0,2);
          lcd.print(myIP); 
          lcd.setCursor(0,1);
          lcd.print(_passwordAP);
          lcd.setCursor(16,3);
          lcd.print(WiFi.RSSI());
        }
        server.send(200, "text/plain", "IP");
        break;
      }
  
      case '2':
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("BME280");
        lcd.setCursor(0,1);
        lcd.print("T: "); lcd.print(BME280_celsTemp,1); lcd.print((char)223); lcd.print("C ");
        lcd.setCursor(0,2);
        lcd.print("H: "); lcd.print(BME280_humidity,1); lcd.print(" %");
        lcd.setCursor(0,3);
        lcd.print("P: "); lcd.print(BME280_pressure,1); lcd.print(" mmHg");
        lcd.setCursor(16,3);
        lcd.print(WiFi.RSSI());
        break;      
      }
  
      case '3':
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("CCS811");
        lcd.setCursor(0,1);
        lcd.print("eCO2: "); lcd.print(CCS811_eCO2,1); lcd.print(" ppm");
        lcd.setCursor(0,2);
        lcd.print("TVOC: "); lcd.print(CCS811_TVOC,1); lcd.print(" ppb");
        lcd.setCursor(16,3);
        lcd.print(WiFi.RSSI());
        break;    
      }
  
      case '4':
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("MH-Z19B");
        lcd.setCursor(0,1);
        lcd.print("CO2: "); lcd.print(MHZ19_CO2,1); lcd.print(" ppm");
        lcd.setCursor(16,3);
        lcd.print(WiFi.RSSI());
        break;      
      }
      
      case '5':
      {
        switch(state1)
        {
          case 0:
          {
            if(net)
            {
              lcd.clear();      
              lcd.setCursor(0,0);
              lcd.print(_ssid);
              lcd.setCursor(0,1);
              lcd.print(myIP);
              lcd.setCursor(16,3);
              lcd.print(WiFi.RSSI());
            }
            else
            {
              lcd.clear();      
              lcd.setCursor(0,0);
              lcd.print("AP:");lcd.print(_ssidAP);
              lcd.setCursor(0,2);
              lcd.print(myIP); 
              lcd.setCursor(0,1);
              lcd.print(_passwordAP);
              lcd.setCursor(16,3);
              lcd.print(WiFi.RSSI());
            }
            break;
          }
          
          case 1:
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("BME280");
            lcd.setCursor(0,1);
            lcd.print("T: "); lcd.print(BME280_celsTemp,1); lcd.print((char)223); lcd.print("C ");
            lcd.setCursor(0,2);
            lcd.print("H: "); lcd.print(BME280_humidity,1); lcd.print(" %");
            lcd.setCursor(0,3);
            lcd.print("P: "); lcd.print(BME280_pressure,1); lcd.print(" mmHg");
            lcd.setCursor(16,3);
            lcd.print(WiFi.RSSI());
            break;
          }
  
          case 2:
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("MH-Z19B");
            lcd.setCursor(0,1);
            lcd.print("CO2: "); lcd.print(MHZ19_CO2,1); lcd.print(" ppm");
            lcd.setCursor(16,3);
            lcd.print(WiFi.RSSI());
            break;
          }
  
          case 3:
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("CCS811");
            lcd.setCursor(0,1);
            lcd.print("eCO2: "); lcd.print(CCS811_eCO2,1); lcd.print(" ppm");
            lcd.setCursor(0,2);
            lcd.print("TVOC: "); lcd.print(CCS811_TVOC,1); lcd.print(" ppb");
            lcd.setCursor(16,3);
            lcd.print(WiFi.RSSI());
            state=0;
            break;
          }
          default:
          {
            state1=0;
            break;
          }
        }
        server.send(200, "text/plain", "Всё");
        break;
      }
      
      default:
      {
        server.send(200, "text/plain", "...");
        break;
      }
    }
    state1++; if(state1>3) state1=0;
    state++;if(state>2) state=0;
  }server.handleClient();
}
