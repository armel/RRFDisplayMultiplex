/*
   RRFTracker Multiplex
   F4HWN Armel
*/

#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>

#define TCAADDR 0x70 // I2C Multiplexer ADDR
#define LED_KO 5
#define LED_UP 2
#define LED_OK 4

// wifi parameter

const char* ssid     = "F4HWN";
const char* password = "hamspirit4ever";

// JSOn endpoint

String endpoint[] = {
  "http://rrf.f5nlg.ovh:8080/RRFTracker/RRF-today/rrf_tiny.json",
  "http://rrf.f5nlg.ovh:8080/RRFTracker/TECHNIQUE-today/rrf_tiny.json",
  "http://rrf.f5nlg.ovh:8080/RRFTracker/BAVARDAGE-today/rrf_tiny.json",
  "http://rrf.f5nlg.ovh:8080/RRFTracker/LOCAL-today/rrf_tiny.json"
};

// misceleanous 

int counter = 1;

struct display {
  U8G2 *display;
  uint8_t i2cnum;
  const u8g2_cb_t *rotation;
  uint16_t position;
  uint8_t width;
  uint8_t height;
};

// all display types, add more if required, give them explicit names, then add them to the OLEDS 'display' array

U8G2_SH1106_128X64_NONAME_F_HW_I2C display128x64(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display128x32(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C


// how many screens are connected to the TCA I2C Multiplexer, min 2, max 8

#define NUMSCREENS 4

// display array, order matters, NUMSCREENS value matters too :-)
// put them in the same order as on the TCA
// (+) it's okay to mix different screen types as long as they're
// supported by u8g2 and their width/height don't exceed MATRIX_SIZE

display OLEDS[NUMSCREENS] = {
  /*{ &type, TCA I2C index, orientation, shared },*/
  { &display128x64, 0, U8G2_R3},
  { &display128x64, 1, U8G2_R3},
  { &display128x64, 2, U8G2_R3},
  { &display128x64, 3, U8G2_R3}
};


// I2C multiplexer controls

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup()   {
  Serial.begin(115200);
  delay(10);

  // we start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // debug trace

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  for (uint8_t i = 0; i < NUMSCREENS; i++) {
    tcaselect(OLEDS[i].i2cnum);
    OLEDS[i].display->begin();
  }
  pinMode (LED_KO, OUTPUT);
  pinMode (LED_UP, OUTPUT);
  pinMode (LED_OK, OUTPUT);
}

void loop() {
  HTTPClient http;
  DynamicJsonDocument doc(8192);

  String tmp_str;

  const char *salon, *date, *indicatif, *emission;
  const char *last_h_1, *last_c_1, *last_d_1;
  const char *last_h_2, *last_c_2, *last_d_2;
  const char *last_h_3, *last_c_3, *last_d_3;
  const char *legende[] = {"00", "06", "12", "18", "23"};
  char hour[5] = {0};
  int tot = 0, link = 0, tx_total = 0, max_level = 0;
  int tx[24] = {0};
  
  if ((WiFi.status() == WL_CONNECTED)) { //check the current connection status

    for (uint8_t i = 0; i < NUMSCREENS; i++) {
            
      http.begin(endpoint[i]); //specify the URL
      http.setTimeout(750);
      int httpCode = http.GET();  //make the request

      if (httpCode > 0) { //check for the returning code

        String payload = http.getString();
        
        // deserialize the JSON document
        DeserializationError error = deserializeJson(doc, payload);

        // test if parsing succeeds
        if (error) {
          digitalWrite(LED_OK, LOW);
          digitalWrite(LED_KO, HIGH);
          Serial.print("Ecran ");
          Serial.print(OLEDS[i].i2cnum);
          Serial.println(" - DeserializeJson() failed");
          continue;
        }
        else {
          digitalWrite(LED_OK, HIGH);
          digitalWrite(LED_KO, LOW);

          salon = doc["abstract"][0]["Salon"];
          date = doc["abstract"][0]["Date"];
          emission = doc["abstract"][0]["Emission cumulée"];
          link = doc["abstract"][0]["Links connectés"];
          tx_total = doc["abstract"][0]["TX total"]; 
          
          indicatif = doc["transmit"][0]["Indicatif"];
          tot = doc["transmit"][0]["TOT"];

          last_h_1 = doc["last"][0]["Heure"];
          last_c_1 = doc["last"][0]["Indicatif"];
          last_d_1 = doc["last"][0]["Durée"];

          last_h_2 = doc["last"][1]["Heure"];
          last_c_2 = doc["last"][1]["Indicatif"];
          last_d_2 = doc["last"][1]["Durée"];

          last_h_3 = doc["last"][2]["Heure"];
          last_c_3 = doc["last"][2]["Indicatif"];
          last_d_3 = doc["last"][2]["Durée"];

          max_level = 0;
          for (uint8_t i = 0; i < 24; i++) {
            int tmp = doc["activity"][i]["TX"];
            if (tmp > max_level) {
              max_level = tmp;
            }
            tx[i] = tmp;
          }
        }
      }
      else {
        digitalWrite(LED_OK, LOW);
        digitalWrite(LED_KO, HIGH);
        Serial.print("Ecran ");
        Serial.print(OLEDS[i].i2cnum);
        Serial.print(" - Error on HTTP request -> ");
        Serial.println(httpCode);
        continue;
      }

      http.end(); //free the resources

      tcaselect(OLEDS[i].i2cnum);
      OLEDS[i].display->firstPage();
      OLEDS[i].display->setFontMode(1);
      OLEDS[i].display->setDrawColor(1);
      OLEDS[i].display->drawBox(0, 1, 128, 8);
      OLEDS[i].display->setDrawColor(0);
      
      do {
        if(counter < 10) {
            // salon
            OLEDS[i].display->setFont(u8g2_font_profont10_tf);
            OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(salon)) / 2 , 8);
            OLEDS[i].display->print(salon);
        }
        else if(counter < 13) {           
            // RRFTracker                        
            OLEDS[i].display->setFont(u8g2_font_profont10_tf);
            OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width("RRFTracker")) / 2 , 8);
            OLEDS[i].display->print("RRFTRACKER");
        }
        else if(counter < 16) {           
            // tx total
            tmp_str = String(tx_total);
            tmp_str = "TX TOTAL " + tmp_str;
                        
            OLEDS[i].display->setFont(u8g2_font_profont10_tf);
            OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(tmp_str.c_str())) / 2 , 8);
            OLEDS[i].display->print(tmp_str);
        }
        else if(counter < 19) {           
            // link total
            tmp_str = String(link);
            tmp_str = "LINKS TOTAL " + tmp_str;
                        
            OLEDS[i].display->setFont(u8g2_font_profont10_tf);
            OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(tmp_str.c_str())) / 2 , 8);
            OLEDS[i].display->print(tmp_str);
        }
        else if(counter < 22) {           
            // BF total
            tmp_str = String(emission);
            tmp_str = "BF TOTAL " + tmp_str;
                      
            OLEDS[i].display->setFont(u8g2_font_profont10_tf);
            OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(tmp_str.c_str())) / 2 , 8);
            OLEDS[i].display->print(tmp_str);
        }
     
        // hour
        
        int len = strlen(date);
        OLEDS[i].display->setFont(u8g2_font_blipfest_07_tr);
        OLEDS[i].display->setCursor(OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(&date[len - 5]) - 1, 8);
        OLEDS[i].display->print(&date[len - 5]);

        OLEDS[i].display->setDrawColor(1);

        for (uint8_t j = 0; j < 127; j += 2) {
          OLEDS[i].display->drawLine(j, 0, j, 0);
          OLEDS[i].display->drawLine(j, 9, j, 9);
          OLEDS[i].display->drawLine(j, 35, j, 35);
        }
        
        // last

        OLEDS[i].display->setFont(u8g2_font_blipfest_07_tr);

        strncpy (hour, last_h_1, 5);
        OLEDS[i].display->setCursor(OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(hour) - 1, 18);
        OLEDS[i].display->print(hour);
        OLEDS[i].display->setCursor(15, 18);
        OLEDS[i].display->print("1");
        
        strncpy (hour, last_h_2, 5);
        OLEDS[i].display->setCursor(OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(hour) - 1, 26);
        OLEDS[i].display->print(hour);
        OLEDS[i].display->setCursor(15, 26);
        OLEDS[i].display->print("2");
        
        strncpy (hour, last_h_3, 5);
        OLEDS[i].display->setCursor(OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(hour) - 1, 34);
        OLEDS[i].display->print(hour);
        OLEDS[i].display->setCursor(15, 34);
        OLEDS[i].display->print("2");

        
        OLEDS[i].display->setFont(u8g2_font_profont10_tf);

        tmp_str = String(last_c_1);
        tmp_str.replace("(", "");
        tmp_str.replace(")", "");
        if (tot != 0) {
          tmp_str = ">" + tmp_str + "<";
        }
        OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(tmp_str.c_str())) / 2 , 18);
        OLEDS[i].display->print(tmp_str);

        tmp_str = String(last_c_2);
        tmp_str.replace("(", "");
        tmp_str.replace(")", "");
        OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(tmp_str.c_str())) / 2 , 26);
        OLEDS[i].display->print(tmp_str);

        tmp_str = String(last_c_3);
        tmp_str.replace("(", "");
        tmp_str.replace(")", "");
        OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(tmp_str.c_str())) / 2 , 34);
        OLEDS[i].display->print(tmp_str);
               
        // bottom screen

        if (tot != 0) { // if transmit
          digitalWrite(LED_UP, HIGH);
          OLEDS[i].display->setContrast(255);
   
          //OLEDS[i].display->setFont(u8g2_font_profont29_tf);
          //OLEDS[i].display->setFont(u8g2_font_bubble_tn);
          OLEDS[i].display->setFont(u8g2_font_luBS18_tn);
          OLEDS[i].display->setCursor((OLEDS[i].display->getDisplayWidth() - OLEDS[i].display->getUTF8Width(last_d_1)) / 2 , 58);
          OLEDS[i].display->print(last_d_1);
        }
        else {  // else histogram
          digitalWrite(LED_UP, LOW);
          OLEDS[i].display->setContrast(16);
          
          int x = 4;
          int tmp = 0;
          
          for (uint8_t j = 0; j < 24; j++) {
            if (tx[j] != 0) {
              tmp = map(tx[j], 0, max_level, 0, 18);
              OLEDS[i].display->drawBox(x, 55 - tmp, 3, tmp);
            }
            x += 5;
          }
  
          for (uint8_t j = 4; j < 124; j += 5) {
            OLEDS[i].display->drawLine(j, 56, j+3, 56);
          }

          OLEDS[i].display->setFont(u8g2_font_blipfest_07_tr);
          for (uint8_t j = 0; j < 5; j++) {
            OLEDS[i].display->setCursor(j * 30, 63);
            OLEDS[i].display->print(legende[j]);
          }
        }
      } while ( OLEDS[i].display->nextPage() );
    }
  }

  // counter for change
  
  counter++;
  if (counter == 22) {
    counter = 1;
  }
}
