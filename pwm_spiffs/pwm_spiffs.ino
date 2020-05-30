#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "RBD_Capacitance.h"
#include "filters.h"

#include "config.h"

const int pwmpin = 14;
const int anapin = 34;

RBD::Capacitance cap_sensor(2, 15);

const float cutoff_freq   = 600.0;  //Cutoff frequency in Hz
const float sampling_time = 0.00005; //Sampling time in seconds.
IIR::ORDER  order  = IIR::ORDER::OD4; // Order (OD1 to OD4)

// Low-pass filter
Filter f(cutoff_freq, sampling_time, order);

int pwm_ch = 0;
int resolution = 1;

int ana_val;
unsigned long capa_val;

// Replace with your network credentials
const char* ssid = "MikroTik";
const char* password = "";

WebSocketsServer webSocket = WebSocketsServer(socket_port);
uint8_t client_id;
char msg_buf[10];

// Create AsyncWebServer object on port 80
AsyncWebServer webServer(80);

void onMessageReceived(String msg) {
  String msg_type = msg.substring(0, 4);
  String msg_data = msg.substring(5);
  Serial.print("msg type: ");
  Serial.print(msg_type);
  Serial.print(" msg data: ");
  Serial.println(msg_data);

  if ( msg_type == "wm00" ) {
    mode = 0;
    saveConfig();
    webSocket.sendTXT(client_id, "am00");
  } else if ( msg_type == "wm01" ) {
    mode = 1;
    saveConfig();
    webSocket.sendTXT(client_id, "am01");
  } else if ( msg_type == "wm10" ) {
    int new_val = msg_data.toInt();
    if ( new_val < 0 || new_val >= max_freq ) {
      webSocket.sendTXT(client_id, "am10:failed");
      webSocket.sendTXT(client_id, "am52:" + String(min_freq));
      return;
    }

    min_freq = new_val;
    saveConfig();
    webSocket.sendTXT(client_id, "am10:ok");
    webSocket.sendTXT(client_id, "am50:" + make_style(min_freq));
  } else if ( msg_type == "wm11" ) {
    int new_val = msg_data.toInt();
    if ( new_val > allowed_freq || new_val <= min_freq ) {
      webSocket.sendTXT(client_id, "am11:failed");
      webSocket.sendTXT(client_id, "am53:" + String(max_freq));
      return;
    }

    max_freq = new_val;
    saveConfig();
    webSocket.sendTXT(client_id, "am11:ok");
    webSocket.sendTXT(client_id, "am51:" + make_style(max_freq));
      webSocket.sendTXT(client_id, "am53:" + String(max_freq));

    ledcSetup(pwm_ch, max_freq, resolution);
  } else if ( msg_type == "wm12" ) {
    if ( capa_val > max_capa  ) {
      webSocket.sendTXT(client_id, "am12:failed");
      return;
    }

    min_capa = capa_val;
    saveConfig();
    webSocket.sendTXT(client_id, "am12:ok");
    webSocket.sendTXT(client_id, "am54:" + String(min_capa));
  } else if ( msg_type == "wm13" ) {
    if ( capa_val < min_capa  ) {
      webSocket.sendTXT(client_id, "am13:failed");
      return;
    }

    max_capa = capa_val;
    saveConfig();
    webSocket.sendTXT(client_id, "am13:ok");
    webSocket.sendTXT(client_id, "am55:" + String(max_capa));
  }
}

void onWebSocketEvent(uint8_t client_num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {

  // Figure out the type of WebSocket event
  switch(type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client_num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
        Serial.printf("[%u] Connection from ", client_num);
        Serial.println(ip.toString());

        client_id = client_num;
        //mode = 0;
        if ( mode == 0 ) {
          webSocket.sendTXT(client_id, "am00");
        } else if ( mode == 1 ) {
          webSocket.sendTXT(client_id, "am01");
        }

         webSocket.sendTXT(client_num, "am50:" + make_style(min_freq));
         webSocket.sendTXT(client_num, "am51:" + make_style(max_freq));
         webSocket.sendTXT(client_num, "am52:" + String(min_freq));
         webSocket.sendTXT(client_num, "am53:" + String(max_freq));
         webSocket.sendTXT(client_num, "am54:" + String(min_capa));
         webSocket.sendTXT(client_num, "am55:" + String(max_capa));
      }
      break;

    // Handle text messages from client
    case WStype_TEXT:
      {
        String msg = (char*)payload;

        onMessageReceived(msg);
      }

      break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Starting...");

  // initialize EEPROM with predefined size
  EEPROM.begin(20);
  
  loadConfig();
  //saveConfig();

  ledcSetup(pwm_ch, min_freq, resolution);
  ledcAttachPin(pwmpin, pwm_ch);

  cap_sensor.setSampleSize(50);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  my_ip = WiFi.localIP().toString();
  Serial.println(my_ip);

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  webServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to load icon file
  webServer.on("/icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/icon.png", "image/png");
  });

  // Start web server
  webServer.begin();

}

int ana_count = 0;

void loop() {
  if ( mode == 0 ) {
    //Serial.println("ana mode");
    ana_val = analogRead(anapin);
    int pwm_val = anatofreq(ana_val);
    //int led_val = mapping_ana(ana_val * (32768.0 / 4096.0));
    //Serial.print("ana mode pwm_val: ");
    //Serial.println(pwm_val);
    ledcSetup(pwm_ch, pwm_val, resolution);
    ledcWrite(pwm_ch, 1);

    ana_count = (ana_count+1) % 5;
    if ( ana_count == 0 ) {
      webSocket.sendTXT(client_id, "am56:" + String(ana_val) + "&" + String(pwm_val));
    }
    delay(100);

  } else {
    //Serial.println("capa mode");
    cap_sensor.update();
    if(cap_sensor.onChange()) {
      // code only runs once per event
      capa_val = cap_sensor.getValue();
      Serial.print("capa_val: ");
      Serial.println(capa_val);
      float filteredval = f.filterIn(capa_val);
      capa_val = filteredval;
      int pwm_val = capatofreq(capa_val);
      
      ledcSetup(pwm_ch, pwm_val, resolution);
      ledcWrite(pwm_ch, 1);
      Serial.print("filter_val: ");
      Serial.println(filteredval);
      Serial.print("capa mode pwm_val: ");
      Serial.println(pwm_val);
      ana_count = (ana_count+1) % 5;
      if ( ana_count == 0 ) {
        webSocket.sendTXT(client_id, "am57:" + String(filteredval) + "&" + String(pwm_val));
      }
    }
  }

  // Look for and handle WebSocket data
  webSocket.loop();
}
