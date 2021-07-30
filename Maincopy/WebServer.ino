void WebServerInit()
{
    // Filesystem status
  server.on("/status", HTTP_GET, handleStatus);

  // List directory
  server.on("/list", HTTP_GET, handleFileList);

  // Load editor
  server.on("/edit", HTTP_GET, handleGetEdit);

  // Create file
  server.on("/edit",  HTTP_PUT, handleFileCreate);

  // Delete file
  server.on("/edit",  HTTP_DELETE, handleFileDelete);

  // Upload file
  // - first callback is called after the request has ended with all parsed arguments
  // - second callback handles file upload at that location
  server.on("/edit",  HTTP_POST, replyOK, handleFileUpload);

  // Default handler for all URIs not defined above
  // Use it to read files from filesystem
  server.onNotFound(handleNotFound);
  server.on("/setMode", handleMode);              //режим отображения экрана
  server.on("/readADC", sensor_data);             //опрос датчиков
  server.on("/setLED", handleLED);                //управление подсветкой экрана
  server.on("/configs.json", handle_ConfigJSON); // формирование configs.json страницы для передачи данных в web интерфейс
  // API для устройства
  server.on("/ssid", handle_Set_Ssid);          // Установить имя и пароль роутера по запросу вида /ssid?ssid=home2&password=12345678
  server.on("/ssidap", handle_Set_Ssidap);      // Установить имя и пароль для точки доступа по запросу вида /ssidap?ssidAP=home1&passwordAP=8765439
  // Start server
  server.on("/restart", handle_Restart);        // Перезагрузка модуля по запросу вида /restart?device=ok
  server.on("/status.json", handle_Status);     //Статус (особо не используется)
  httpUpdater.setup(&server);
  server.begin();
  lcd.setCursor(0,1);
  lcd.print("");
  DBG_OUTPUT_PORT.println("HTTP server started");
}

void handleMode()
{
  LCDmode = server.arg("mode");
  char c=LCDmode[0];
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
      server.send(200, "text/plain", "BME280");      
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
      server.send(200, "text/plain", "CCS811");      
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
      server.send(200, "text/plain", "MH-Z19B");      
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
}


void handleLED()
{
  if (server.arg("LEDstate")=="1")
  {
    lcd.backlight();
    server.send(200, "text/plain", "ВКЛ");
  }
  else if (server.arg("LEDstate")=="0")
  {
    lcd.noBacklight();
    server.send(200, "text/plain", "ВЫКЛ");
  }
}
void sensor_data()
{
    server.send(200,"application/json",getStatus());
}

// Установка параметров для подключения к внешней AP
void handle_Set_Ssid() {
  _ssid = server.arg("ssid");            // Получаем значение ssid из запроса сохраняем в глобальной переменной
  _password = server.arg("password");    // Получаем значение password из запроса сохраняем в глобальной переменной
  saveConfig();                        // Функция сохранения данных во Flash
  server.send(200, "text/plain", "OK");   // отправляем ответ о выполнении
}
//Установка параметров внутренней точки доступа
void handle_Set_Ssidap() {              //
  _ssidAP = server.arg("ssidAP");         // Получаем значение ssidAP из запроса сохраняем в глобальной переменной
  _passwordAP = server.arg("passwordAP"); // Получаем значение passwordAP из запроса сохраняем в глобальной переменной
  saveConfig();                         // Функция сохранения данных во Flash
  server.send(200, "text/plain", "OK");   // отправляем ответ о выполнении
}

void handle_ConfigJSON() {
  String root = "{}";  // Формировать строку для отправки в браузер json формат
  //{"SSDP":"SSDP-test","ssid":"home","password":"i12345678","ssidAP":"WiFi","passwordAP":"","ip":"192.168.0.101"}
  // Резервируем память для json обекта буфер может рости по мере необходимти, предпочтительно для ESP8266
  DynamicJsonBuffer jsonBuffer;
  //  вызовите парсер JSON через экземпляр jsonBuffer
  JsonObject& json = jsonBuffer.parseObject(root);
  // Заполняем поля json
  json["ssidAPName"] = _ssidAP;
  json["ssidAPPassword"] = _passwordAP;
  json["ssidName"] = _ssid;
  json["ssidPassword"] = _password;

  // Помещаем созданный json в переменную root
  root = "";
  json.printTo(root);
  server.send(200, "text/json", root);
}

void handle_Status() {
  String root = "{}"; 
  DynamicJsonBuffer jsonBuffer;
  //  вызовите парсер JSON через экземпляр jsonBuffer
  JsonObject& json = jsonBuffer.parseObject(root);
  // Заполняем поля json
  json["net"] = net;
  json["BME280"] = _BME280;
  json["CCS811"] = _CCS811;
  json["MHZ19B"] = MHZ19B;
  // Помещаем созданный json в переменную root
  root = "";
  json.printTo(root);
  server.send(200, "text/json", root);
}

void handle_Restart() {
  String restart = server.arg("device");          // Получаем значение device из запроса
  if (restart == "ok") {                         // Если значение равно Ок
    server.send(200, "text / plain", "Reset OK"); // Oтправляем ответ Reset OK
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reboot");
    ESP.restart();                                // перезагружаем модуль
  }
  else {                                        // иначе
    server.send(200, "text / plain", "No Reset"); // Oтправляем ответ No Reset
  }
}

//Обработка запросов при работе файловой системы
void handleStatus() {
  DBG_OUTPUT_PORT.println("handleStatus");
  FSInfo fs_info;
  String json;
  json.reserve(128);

  json = "{\"type\":\"";
  json += fsName;
  json += "\", \"isOk\":";
  if (fsOK) {
    fileSystem->info(fs_info);
    json += F("\"true\", \"totalBytes\":\"");
    json += fs_info.totalBytes;
    json += F("\", \"usedBytes\":\"");
    json += fs_info.usedBytes;
    json += "\"";
  } else {
    json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";

  server.send(200, "application/json", json);
}


/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chuncked responses.
*/
void handleFileList() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  if (!server.hasArg("dir")) {
    return replyBadRequest(F("DIR ARG MISSING"));
  }

  String path = server.arg("dir");
  if (path != "/" && !fileSystem->exists(path)) {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileList: ") + path);
  Dir dir = fileSystem->openDir(path);
  path.clear();

  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!server.chunkedResponseModeStart(200, "text/json")) {
    server.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }

  // use the same string for every line
  String output;
  output.reserve(64);
  while (dir.next()) {
#ifdef USE_SPIFFS
    String error = checkForUnsupportedPath(dir.fileName());
    if (error.length() > 0) {
      DBG_OUTPUT_PORT.println(String("Ignoring ") + error + dir.fileName());
      continue;
    }
#endif
    if (output.length()) {
      // send string from previous iteration
      // as an HTTP chunk
      server.sendContent(output);
      output = ',';
    } else {
      output = '[';
    }

    output += "{\"type\":\"";
    if (dir.isDirectory()) {
      output += "dir";
    } else {
      output += F("file\",\"size\":\"");
      output += dir.fileSize();
    }

    output += F("\",\"name\":\"");
    // Always return names without leading "/"
    if (dir.fileName()[0] == '/') {
      output += &(dir.fileName()[1]);
    } else {
      output += dir.fileName();
    }

    output += "\"}";
  }

  // send last string
  output += "]";
  server.sendContent(output);
  server.chunkedResponseFinalize();
}


/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println(String("handleFileRead: ") + path);
  if (!fsOK) {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  if (path.endsWith("/")) {
    path += "index.htm";
  }

  String contentType;
  if (server.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
    File file = fileSystem->open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      DBG_OUTPUT_PORT.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}


/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
String lastExistingParent(String path) {
  while (!path.isEmpty() && !fileSystem->exists(path)) {
    if (path.lastIndexOf('/') > 0) {
      path = path.substring(0, path.lastIndexOf('/'));
    } else {
      path = String();  // No slash => the top folder does not exist
    }
  }
  DBG_OUTPUT_PORT.println(String("Last existing parent: ") + path);
  return path;
}

/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
void handleFileCreate() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg("path");
  if (path.isEmpty()) {
    return replyBadRequest(F("PATH ARG MISSING"));
  }

#ifdef USE_SPIFFS
  if (checkForUnsupportedPath(path).length() > 0) {
    return replyServerError(F("INVALID FILENAME"));
  }
#endif

  if (path == "/") {
    return replyBadRequest("BAD PATH");
  }
  if (fileSystem->exists(path)) {
    return replyBadRequest(F("PATH FILE EXISTS"));
  }

  String src = server.arg("src");
  if (src.isEmpty()) {
    // No source specified: creation
    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
    if (path.endsWith("/")) {
      // Create a folder
      path.remove(path.length() - 1);
      if (!fileSystem->mkdir(path)) {
        return replyServerError(F("MKDIR FAILED"));
      }
    } else {
      // Create a file
      File file = fileSystem->open(path, "w");
      if (file) {
        file.write((const char *)0);
        file.close();
      } else {
        return replyServerError(F("CREATE FAILED"));
      }
    }
    if (path.lastIndexOf('/') > -1) {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    replyOKWithMsg(path);
  } else {
    // Source specified: rename
    if (src == "/") {
      return replyBadRequest("BAD SRC");
    }
    if (!fileSystem->exists(src)) {
      return replyBadRequest(F("SRC FILE NOT FOUND"));
    }

    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

    if (path.endsWith("/")) {
      path.remove(path.length() - 1);
    }
    if (src.endsWith("/")) {
      src.remove(src.length() - 1);
    }
    if (!fileSystem->rename(src, path)) {
      return replyServerError(F("RENAME FAILED"));
    }
    replyOKWithMsg(lastExistingParent(src));
  }
}


/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive(String path) {
  File file = fileSystem->open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir) {
    fileSystem->remove(path);
    return;
  }

  // Otherwise delete its contents first
  Dir dir = fileSystem->openDir(path);

  while (dir.next()) {
    deleteRecursive(path + '/' + dir.fileName());
  }

  // Then delete the folder itself
  fileSystem->rmdir(path);
}


/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg(0);
  if (path.isEmpty() || path == "/") {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileDelete: ") + path);
  if (!fileSystem->exists(path)) {
    return replyNotFound(FPSTR(FILE_NOT_FOUND));
  }
  deleteRecursive(path);

  replyOKWithMsg(lastExistingParent(path));
}

/*
   Handle a file upload request
*/
void handleFileUpload() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
    uploadFile = fileSystem->open(filename, "w");
    if (!uploadFile) {
      return replyServerError(F("CREATE FAILED"));
    }
    DBG_OUTPUT_PORT.println(String("Upload: START, filename: ") + filename);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize) {
        return replyServerError(F("WRITE FAILED"));
      }
    }
    DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    DBG_OUTPUT_PORT.println(String("Upload: END, Size: ") + upload.totalSize);
  }
}


/*
   The "Not Found" handler catches all URI not explicitely declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

  if (handleFileRead(uri)) {
    return;
  }

  // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += uri;
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += '\n';
  for (uint8_t i = 0; i < server.args(); i++) {
    message += F(" NAME:");
    message += server.argName(i);
    message += F("\n VALUE:");
    message += server.arg(i);
    message += '\n';
  }
  message += "path=";
  message += server.arg("path");
  message += '\n';
  DBG_OUTPUT_PORT.print(message);

  return replyNotFound(message);
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
void handleGetEdit() {
  if (handleFileRead(F("/edit/index.htm"))) {
    return;
  }

#ifdef INCLUDE_FALLBACK_INDEX_HTM
  server.sendHeader(F("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
#else
  replyNotFound(FPSTR(FILE_NOT_FOUND));
#endif

}
