/*
  FSBrowser - A web-based FileSystem Browser for ESP8266 filesystems

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  See readme.md for more information.
*/

////////////////////////////////

// Select the FileSystem by uncommenting one of the lines below

#define USE_SPIFFS
/*
////////////////////////////////

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
*/
#if defined USE_SPIFFS
#include <FS.h>
const char* fsName = "SPIFFS";
FS* fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif


#define DBG_OUTPUT_PORT Serial

static bool fsOK;
String unsupportedFiles = String();

File uploadFile;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK() {
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  DBG_OUTPUT_PORT.println(msg);
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  DBG_OUTPUT_PORT.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

#ifdef USE_SPIFFS
/*
   Checks filename for character combinations that are not supported by FSBrowser (alhtough valid on SPIFFS).
   Returns an empty String if supported, or detail of error(s) if unsupported
*/
String checkForUnsupportedPath(String filename) {
  String error = String();
  if (!filename.startsWith("/")) {
    error += F("!NO_LEADING_SLASH! ");
  }
  if (filename.indexOf("//") != -1) {
    error += F("!DOUBLE_SLASH! ");
  }
  if (filename.endsWith("/")) {
    error += F("!TRAILING_SLASH! ");
  }
  return error;
}
#endif


////////////////////////////////
// Request handlers

/*
   Return the FS type, status and size info
*/


void FSInit(void) {
  ////////////////////////////////
  // SERIAL INIT
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print('\n');

  ////////////////////////////////
  // FILESYSTEM INIT

  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  DBG_OUTPUT_PORT.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));

#ifdef USE_SPIFFS
  // Debug: dump on console contents of filessytem with no filter and check filenames validity
  Dir dir = fileSystem->openDir("");
  DBG_OUTPUT_PORT.println(F("List of files at root of filesystem:"));
  while (dir.next()) {
    String error = checkForUnsupportedPath(dir.fileName());
    String fileInfo = dir.fileName() + (dir.isDirectory() ? " [DIR]" : String(" (") + dir.fileSize() + "b)");
    DBG_OUTPUT_PORT.println(error + fileInfo);
    if (error.length() > 0) {
      unsupportedFiles += error + fileInfo + '\n';
    }
  }
  DBG_OUTPUT_PORT.println();

  // Keep the "unsupportedFiles" variable to show it, but clean it up
  unsupportedFiles.replace("\n", "<br/>");
  unsupportedFiles = unsupportedFiles.substring(0, unsupportedFiles.length() - 5);
#endif
}
