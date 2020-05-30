#include <EEPROM.h>

const int allowed_freq = 5 * 1000 * 1000;

const unsigned long DEFAULT_MIN_FREQ =  1 * 1000;
const unsigned long DEFAULT_MAX_FREQ = 10 * 1000;

const unsigned long DEFAULT_MIN_CAPA =  100;
const unsigned long DEFAULT_MAX_CAPA = 1000;

unsigned long min_freq = 0;// = 1.2 * 1000;
unsigned long max_freq = 0;// =  12 * 1000;

unsigned long min_capa = 0;// =   1 * 1000;
unsigned long max_capa = 0;// =   4 * 1000;

const unsigned int socket_port = 1337;
String my_ip;

int mode = 0;   // 0 - ana to pwm
                // 1 - capa to pwm


void EEPROM16_Write(uint8_t a, uint16_t b){
  EEPROM.write(a, lowByte(b));
  EEPROM.write(a + 1, highByte(b));
}

uint16_t EEPROM16_Read(uint8_t a){
  return word(EEPROM.read(a + 1), EEPROM.read(a));
}

//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to adress + 3.
void EEPROMWritelong(int address, long value)
{
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to adress + 3.
long EEPROMReadlong(long address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
  
  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void loadConfig() {
  min_freq = EEPROMReadlong(0);
  max_freq = EEPROMReadlong(4);
  min_capa = EEPROMReadlong(8);
  max_capa = EEPROMReadlong(12);
  mode = EEPROMReadlong(16);
  
  if ( min_freq == 0 )
    min_freq = DEFAULT_MIN_FREQ;

  if ( max_freq == 0 )
    max_freq = DEFAULT_MAX_FREQ;

  if ( min_capa == 0 )
    min_capa = DEFAULT_MIN_CAPA;

  if ( max_capa == 0 )
    max_capa = DEFAULT_MAX_CAPA;
}

void saveConfig() {
  EEPROMWritelong(0, min_freq);
  EEPROMWritelong(4, max_freq);
  EEPROMWritelong(8, min_capa);
  EEPROMWritelong(12, max_capa);
  EEPROMWritelong(16, mode);

  EEPROM.commit();
}

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

int mapping_ana(int ana) {
  float res = 32768.0;
  float min_pwm = res / max_freq * min_freq;

  return min_pwm + (res - min_pwm) * (ana / res);
}

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if ( var == "WEBSOCKETURL" ) {
    String url;
    url = my_ip + ":" + String(socket_port);
    return url;
  }
  return String();
}

int anatofreq(int ana_val) {
  return min_freq + (max_freq - min_freq) / 4096.0 * ana_val;
}

int capatofreq(int capa_val) {
  int ret_val = 0;
  
  if ( capa_val < min_capa ) {
    ret_val = min_freq;

  } else if ( capa_val > max_capa ) {
    ret_val = max_freq;
  } else {
    ret_val = min_freq + (max_freq - min_freq) / ((float)(max_capa - min_capa)) * (capa_val - min_capa);
  }
  //return ret_val;
  return max_freq - (ret_val - min_freq);
}
