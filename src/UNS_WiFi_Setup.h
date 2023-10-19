#ifndef UNS_WIFI_SETUP_H
#define UNS_WIFI_SETUP_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

const char *default_ssid = "@AnwarPanjie";
const char *default_password = "BE123282805";

WebServer server(80);

void uns_soft_AP (void) {
    // soft AP with UNS-Env + efuse mac as ssid
    char ssid[34];
    sprintf(ssid, "UNS-Env-%04X%08X", (uint16_t)(ESP.getEfuseMac() >> 32), (uint32_t)ESP.getEfuseMac());

    WiFi.softAP(ssid, "12345678");

    Serial.printf("[INFO] Soft AP SSID: %s\n", ssid);
    Serial.printf("[INFO] Soft AP Password: 12345678\n");
    Serial.printf("[INFO] IP Address: %s\n", WiFi.softAPIP().toString().c_str());
}

// check if wifi.txt exists and then read ssid and password from it
// if wifi.txt doesn't exists, then connect to default ssid and password
// if default ssid and password is not available, then create soft AP
void uns_alternate_connection (void) {
    if (SPIFFS.exists("/wifi.txt")) {
        File file = SPIFFS.open("/wifi.txt", FILE_READ);

        if (!file) {
            Serial.println("[ERROR] Failed to open file for reading");
            return;
        }

        String ssid = file.readStringUntil('\n');
        String password = file.readStringUntil('\n');

        Serial.printf("[INFO] SSID: %s\n", ssid.c_str());
        Serial.printf("[INFO] Password: %s\n", password.c_str());

        file.close();

        WiFi.begin(ssid.c_str(), password.c_str());

        Serial.printf("[INFO] Connecting to %s\n", ssid.c_str());

        uint32_t elapsed_time = millis();

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");

            if (millis() - elapsed_time > 10000) {
                break;
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("\n[INFO] Connected to %s\n", ssid.c_str());
            Serial.printf("[INFO] IP Address: %s\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.printf("\n[ERROR] Failed to connect to %s\n", ssid.c_str());
            uns_soft_AP();
            Serial.println("[INFO] Soft AP setup done!");
        }
    } else {
        Serial.println("[INFO] wifi.txt doesn't exists");
        uns_soft_AP();
        Serial.println("[INFO] Soft AP setup done!");
    }
}

void server_root () {
    String html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "<head>\n";
    html += "<title>UNS-Env</title>\n";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    html += "<link rel=\"icon\" href=\"data:,\">\n";
    html += "<style>\n";
    html += "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }\n";
    html += ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;\n";
    html += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }\n";
    html += ".button2 {background-color: #77878A;}\n";
    html += ".box { background-color: #2196F3; border-radius: 10px; padding: 20px; }\n";
    html += "</style>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "<h1>UNS-Env</h1>\n";
    html += "<div class=\"box\">\n";
    html += "<h2>Temperature</h2>\n";
    html += "<p id=\"temperature\">0</p>\n";
    html += "</div>\n";
    html += "<div class=\"box\">\n";
    html += "<h2>Humidity</h2>\n";
    html += "<p id=\"humidity\">0</p>\n";
    html += "</div>\n";
    html += "<script>\n";
    html += "setInterval(function() {\n";
    html += "getSensorData();\n";
    html += "}, 1000);\n";
    html += "function getSensorData() {\n";
    html += "var xhttp = new XMLHttpRequest();\n";
    html += "xhttp.onreadystatechange = function() {\n";
    html += "if (this.readyState == 4 && this.status == 200) {\n";
    html += "document.getElementById(\"temperature\").innerHTML = this.responseText;\n";
    html += "}\n";
    html += "};\n";
    html += "xhttp.open(\"GET\", \"/temperature\", true);\n";
    html += "xhttp.send();\n";
    html += "var xhttp = new XMLHttpRequest();\n";
    html += "xhttp.onreadystatechange = function() {\n";
    html += "if (this.readyState == 4 && this.status == 200) {\n";
    html += "document.getElementById(\"humidity\").innerHTML = this.responseText;\n";
    html += "}\n";
    html += "};\n";
    html += "xhttp.open(\"GET\", \"/humidity\", true);\n";
    html += "xhttp.send();\n";
    html += "}\n";
    html += "</script>\n";

    // add button to move to wifi setup page
    html += "<p><a href=\"/wifi_setup\"><button class=\"button\">WiFi Setup</button></a></p>\n";
    html += "</body>\n";
    html += "</html>\n";

    server.send(200, "text/html", html);
}

// setup wifi with form and get the value and then apply it into wifi esp32
void server_setup_wifi () {
    String html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "<head>\n";
    html += "<title>UNS-Env</title>\n";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    html += "<link rel=\"icon\" href=\"data:,\">\n";
    html += "<style>\n";
    html += "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }\n";
    html += ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;\n";
    html += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }\n";
    html += ".button2 {background-color: #77878A;}\n";

    // css style for the form with the maximum size
    html += "input[type=text], select {\n";
    html += "width: 100%;\n";
    html += "padding: 12px 20px;\n";
    html += "margin: 8px 0;\n";
    html += "display: inline-block;\n";

    // flat light blue color box
    html += "border: 1px solid #ccc;\n";
    html += "border-radius: 4px;\n";
    html += "box-sizing: border-box;\n";
    html += "}\n";

    // css style for the form
    html += "input[type=password], select {\n";
    html += "width: 100%;\n";
    html += "padding: 12px 20px;\n";
    html += "margin: 8px 0;\n";
    html += "display: inline-block;\n";

    // flat light blue color box
    html += "border: 1px solid #ccc;\n";
    html += "border-radius: 4px;\n";
    html += "box-sizing: border-box;\n";
    html += "}\n";

    // css style for the form
    html += "input[type=submit] {\n";
    html += "width: 100%;\n";

    // flat light blue color box
    html += "background-color: #4CAF50;\n";
    html += "color: white;\n";
    html += "padding: 14px 20px;\n";
    html += "margin: 8px 0;\n";
    html += "border: none;\n";
    html += "border-radius: 4px;\n";
    html += "cursor: pointer;\n";
    html += "}\n";

    // css style for the form
    html += "input[type=submit]:hover {\n";
    
    // flat light blue color box
    html += "background-color: #45a049;\n";
    html += "}\n";

    // css style for the form
    html += "div {\n";
    html += "border-radius: 5px;\n";
    html += "background-color: #f2f2f2;\n";
    html += "padding: 20px;\n";
    html += "}\n";

    // css style for the form
    html += "label {\n";
    html += "padding: 12px 12px 12px 0;\n";
    html += "display: inline-block;\n";
    html += "}\n";

    // css style for the move back button
    html += ".button {\n";
    html += "background-color: #4CAF50;\n";
    html += "border: none;\n";
    html += "color: white;\n";
    html += "padding: 16px 40px;\n";
    html += "text-decoration: none;\n";
    html += "font-size: 30px;\n";
    html += "margin: 2px;\n";
    html += "cursor: pointer;\n";
    html += "}\n";

    html += "</style>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "<h1>UNS-Env</h1>\n";
    html += "<form action=\"/setup_wifi\" method=\"POST\">\n";
    html += "<label for=\"ssid\">SSID:</label><br>\n";
    html += "<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>\n";
    html += "<label for=\"password\">Password:</label><br>\n";
    html += "<input type=\"password\" id=\"password\" name=\"password\"><br><br>\n";
    html += "<input type=\"submit\" value=\"Submit\">\n";

    // add button to move to root page
    html += "<p><a href=\"/\"><button class=\"button\">Back</button></a></p>\n";

    html += "</form>\n";
    html += "</body>\n";
    html += "</html>\n";

    server.send(200, "text/html", html);
}

// handle post request from wifi setup page
void server_setup_wifi_post () {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    Serial.printf("[INFO] SSID: %s\n", ssid.c_str());
    Serial.printf("[INFO] Password: %s\n", password.c_str());

    // html file with OK response and restart info
    // the html file is added with css style
    // with the flat light blue color box

    String html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "<head>\n";
    html += "<title>UNS-Env</title>\n";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    html += "<link rel=\"icon\" href=\"data:,\">\n";
    html += "<style>\n";
    html += "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }\n";
    html += ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;\n";
    html += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }\n";
    html += ".button2 {background-color: #77878A;}\n";
    html += ".box { background-color: #2196F3; border-radius: 10px; padding: 20px; }\n";
    html += "</style>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "<h1>UNS-Env</h1>\n";
    html += "<div class=\"box\">\n";
    html += "<h2>WiFi Setup</h2>\n";
    html += "<p>WiFi setup done!</p>\n";
    html += "<p>Restarting ESP32...</p>\n";
    html += "</div>\n";
    html += "</body>\n";
    html += "</html>\n";

    server.send(200, "text/html", html);

    // save ssid and password to spiffs
    File file = SPIFFS.open("/wifi.txt", FILE_WRITE);

    if (!file) {
        Serial.println("[ERROR] Failed to open file for writing");
        return;
    }

    if (file.print(ssid + "\n" + password)) {
        Serial.println("[INFO] File written");
    } else {
        Serial.println("[ERROR] Write failed");
    }

    file.close();

    // restart esp32
    ESP.restart();
}

void server_temperature () {
    String temperature = String(dht.getTemperature());

    server.send(200, "text/plain", temperature);
}

void server_humidity () {
    String humidity = String(dht.getHumidity());

    server.send(200, "text/plain", humidity);
}

void server_start (void) {
    server.on("/", server_root);
    server.on("/temperature", server_temperature);
    server.on("/humidity", server_humidity);
    server.on("/wifi_setup", server_setup_wifi);
    server.on("/setup_wifi", server_setup_wifi_post);

    server.begin();

    Serial.println("[INFO] Server started!");
}

#endif // UNS_WIFI_SETUP_H