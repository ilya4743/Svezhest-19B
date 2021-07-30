//формирование json с показаниями датчиков
String getStatus()
{
  String reply="";
  reply += "{";
  reply += "\"system\":{ ";
  reply += "\"Akey\":\"" +String(Akey)+ "\",";
  reply += "\"Version\": \"" + String(VER) + "\",";
  reply += "\"RSSI\":" + String(WiFi.RSSI()) + ",";
  reply += "\"MAC\":\"" + String(WiFi.macAddress()) + "\"";
  reply += "}, ";

  reply += "\"MHZ19B\":{ ";
  reply += "\"CO2\":"+String(MHZ19_CO2);
  reply += "}, ";

  reply += "\"CCS811\":{ ";
  reply += "\"eCO2\":"+String(CCS811_eCO2)+",";
  reply += "\"TVOC\":"+String(CCS811_TVOC);
  reply += "}, ";
  
  reply += "\"BME280\":{ ";
  reply += "\"temp\":"+String(BME280_celsTemp)+",";
  reply += "\"humidity\":"+String(BME280_humidity)+",";
  reply += "\"pressure\":" + String(BME280_pressure);
  reply += "}";

  reply += "}";
  return(reply);
}

// Передать данные на сервер и по Serial
void CheckWifi()
{
  String status = getStatus();
  if(WiFi.status() != WL_CONNECTED) // и соединения нет
    Serial.print("."); // ждём соединения
  else
  { 
    // иначе если есть соединение
    // передаём данные
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("Initialization completed");
    Serial.println("Sending data...");
    http.begin("http://188.35.161.31/core/jsonadd.php");
    http.addHeader("Content-Type", "application/json");
    int resulthttp = http.POST(status);
    Serial.print("Done: ");
    Serial.println(resulthttp);
    http.end();
  }
  Serial.println(status);
}

/*Инициализация датчиков и всякого оборудования*/
void SensorsInit()
{
  InitBME280();
  InitCCS811();
  InitMHZ19B();
}

void InitBME280()
{
  unsigned status;
  status = BME280.begin(BME280_ADDR);  
  if (!status) 
  {
    _BME280=false;
     Serial.println("Did not find BME280 sensor!");
     Serial.print("SensorID was: 0x"); Serial.println(BME280.sensorID(),16);
  }
  else
    _BME280=true;
}

void InitMHZ19B()
{
  SerialCO2_MHZ19.begin (9600); 
  byte setrangeA_cmd[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xCB}; // задаёт диапазон 0 - 5000ppm
  unsigned char setrangeA_response[9]; 
  SerialCO2_MHZ19.write(setrangeA_cmd,9);
  SerialCO2_MHZ19.readBytes(setrangeA_response, 9);
  int setrangeA_i;
  byte setrangeA_crc = 0;
  for (setrangeA_i = 1; setrangeA_i < 8; setrangeA_i++) setrangeA_crc+=setrangeA_response[setrangeA_i];
  setrangeA_crc = 255 - setrangeA_crc;
  setrangeA_crc += 1;
  if ( !(setrangeA_response[0] == 0xFF && setrangeA_response[1] == 0x99 && setrangeA_response[8] == setrangeA_crc) )
    Serial.println("MH-Z19: Range CRC error: " + String(setrangeA_crc) + " / "+ String(setrangeA_response[8]) + " (bytes 6 and 7)");
  else 
    Serial.println("MH-Z19: Range was set! (bytes 6 and 7)");
}

void InitCCS811()
{
  if(!CCS811.begin(CCS811_ADDR)){
    Serial.println("Failed to start sensor! Please check your wiring.");
    _CCS811=false;
  }
  else
    _CCS811=true;

  // Wait for the sensor to be ready
  while(!CCS811.available());
}

/*Опрос датчиков и всякого оборудования*/
void getSensors()
{
  getCO2_MHZ19B();
  getBME280();
  getCCS811();
  
}
void getCO2_MHZ19B()
{
  byte measure_cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  unsigned char measure_response[9]; 
  // ***** узнаём концентрацию CO2 через UART: ***** 
  SerialCO2_MHZ19.write(measure_cmd,9);
  SerialCO2_MHZ19.readBytes(measure_response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=measure_response[i];
  crc = 255 - crc;
  crc += 1;
  if ( !(measure_response[0] == 0xFF && measure_response[1] == 0x86 && measure_response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / "+ String(measure_response[8]));     MHZ19_CO2=0;
}
else{ 
  unsigned int responseHigh = (unsigned int) measure_response[2];
  unsigned int responseLow = (unsigned int) measure_response[3];
  MHZ19_CO2 = (256*responseHigh) + responseLow;
  Serial.println("MH-Z19B: CO2 " + String(MHZ19_CO2)+" ppm");}
}

void getBME280() 
{
  BME280_humidity=BME280.readHumidity();
  BME280_celsTemp=BME280.readTemperature();
  BME280_pressure=(BME280.readPressure() / 100.0F)*0.750064;
  Serial.print("BME280:\tTemperature = ");
  Serial.print(BME280_celsTemp);
  Serial.print(" *C\t");
  Serial.print("Humidity = ");
  Serial.print(String(BME280_humidity)+" %");
  Serial.print("Pressure = ");
  Serial.print(BME280_pressure);
  Serial.print(" mm\t");
  Serial.print("Approx. Altitude = ");
  Serial.println(" m\t");
}

void getCCS811()
{
    if(CCS811.available()){
      CCS811.setEnvironmentalData(BME280_humidity,BME280_celsTemp);
    if(!CCS811.readData()){      
      CCS811_eCO2=CCS811.geteCO2();
      CCS811_TVOC=CCS811.getTVOC();
      Serial.print("CO2: ");
      Serial.print((CCS811_eCO2));
      Serial.print("ppm, TVOC: ");
      Serial.println(CCS811_TVOC);
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }
}
