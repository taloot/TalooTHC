// Load Wi-Fi library
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "RBD_Capacitance.h"

#include "config.h"

int mode = 0;   // 0 - ana to pwm
                // 1 - capa to pwm

const int pwmpin = 27;
const int anapin = 34;

const int allowed_freq = 5 * 1000 * 1000;

unsigned long max_freq =  12 * 1000;
unsigned long min_freq = 1.2 * 1000;

unsigned long max_capa =   4 * 1000;
unsigned long min_capa =   1 * 1000;

int pwm_ch = 0;
int resolution = 12;

int ana_val;
unsigned long capa_val;

RBD::Capacitance cap_sensor(19, T0);

// Replace with your network credentials
const char* ssid = "MikroTik";
const char* password = "";

// Set web server port number to 80
AsyncWebServer server(80);

String make_style(int freq) {
  if ( freq >= 1000 * 1000 ) { //MHz
    return String((float)freq / 1000 / 1000) + "MHz";
  } else if ( freq >= 1000 ) { //KHz
    return String((float)freq / 1000) + "KHz";
  } else {
    return String(freq) + "Hz";
  }
  return String();
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "FREQ_MIN_STYLE"){
    return make_style(min_freq);
  }
  if(var == "FREQ_MAX_STYLE"){
    return make_style(max_freq);
  }
  if(var == "MINFREQVALUE"){
    return String(min_freq);
  }
  if(var == "MAXFREQVALUE"){
    return String(max_freq);
  }
  if(var == "CAPA_MIN_STYLE"){
    return String(min_capa);
  }
  if(var == "CAPA_MAX_STYLE"){
    return String(max_capa);
  }
  return String();
}


int mapping_ana(int ana) {
  float res = 4096.0;
  float min_pwm = res / max_freq * min_freq;

  return min_pwm + (res - min_pwm) * (ana / res);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  

  loadConfig();
  
  ledcSetup(pwm_ch, max_freq, resolution);
  ledcAttachPin(pwmpin, pwm_ch);

  //cap_sensor.setSampleSize(500);

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
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/setmin", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("HTTP_GET: setmin");
    String inputMessage;
    
    inputMessage = request->getParam("val")->value();
    int new_val = inputMessage.toInt();

    if ( new_val < 0 || new_val >= max_freq ) {
      request->send(200, "text/plain", "FAILED");
      return;
    }

    min_freq = new_val;
    Serial.print("set min freq: ");
    Serial.println(min_freq);
    request->send(200, "text/plain", make_style(min_freq));
  });

  server.on("/setmax", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("HTTP_GET: setmax");
    String inputMessage;
    
    inputMessage = request->getParam("val")->value();
    int new_val = inputMessage.toInt();

    if ( new_val > allowed_freq || new_val <= min_freq ) {
      request->send(200, "text/plain", "FAILED");
      return;
    }
    
    max_freq = new_val;
    Serial.print("set max freq: ");
    Serial.println(max_freq);
    request->send(200, "text/plain", make_style(max_freq));

    ledcSetup(pwm_ch, max_freq, resolution);
  });

  server.on("/setcapamax", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("HTTP_GET: setcapamax");

    if ( capa_val < min_capa ) {
      request->send(200, "text/plain", "FAILED");
      return;
    }
    
    max_capa = capa_val;
    Serial.print("set max capa: ");
    Serial.println(max_capa);
    request->send(200, "text/plain", String(max_capa));
  });

  server.on("/setcapamin", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("HTTP_GET: setcapamax");

    if ( capa_val > max_capa ) {
      request->send(200, "text/plain", "FAILED");
      return;
    }
    
    min_capa = capa_val;
    Serial.print("set in capa: ");
    Serial.println(min_capa);
    request->send(200, "text/plain", String(min_capa));
  });

  server.on("/setmode", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("HTTP_GET: setmode");
    String inputMessage;
    
    inputMessage = request->getParam("val")->value();
    int new_val = inputMessage.toInt();

    mode = new_val;
    
    if ( mode == 0 ) {
      Serial.println("set analog mode");
    } else if ( mode == 1) {
      Serial.println("set capacitance mode");
    }
    
    request->send(200, "text/plain", "OK");
  });

  server.on("/getcapa", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("HTTP_GET: getcapa");
    request->send(200, "text/plain", String(capa_val));
  });
  
  server.begin();
  //cap_sensor.start();
}

void loop() {
  if ( mode == 0 ) {
    //Serial.println("ana mode");
    ana_val = analogRead(anapin);
    int led_val = mapping_ana(ana_val);
    ledcWrite(pwm_ch, led_val);
    delay(100);

  } else {
    //Serial.println("capa mode");
    cap_sensor.update();
    if(cap_sensor.onChange()) {
      // code only runs once per event
      capa_val = cap_sensor.getValue();
      Serial.print("capa: ");
      Serial.println(capa_val);
    }
  }
  
}
