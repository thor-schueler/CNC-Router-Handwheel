#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <regex>
#include "config_page.h"
#include "../logging/SerialLogger.h"

const char *assid = ACCESS_POINT_NAME;
const char *asecret = ACCESS_POINT_PWD;
const char *PARAM_INPUT_ssid = "SSID";
const char *PARAM_INPUT_psw = "psw";

/**
 * @brief Static web server object.
 * 
 */
AsyncWebServer Config::server = AsyncWebServer(80);

/**
 * @brief Construct a new Config object
 * 
 */
Config::Config()
{
  // initialize the commands array with 12 command slots
  for(int i=0; i<12; i++) this->Commands.emplace(i, Command{String(i),"N " + String(i),"Alt " + String(i),"Alt N " + String(i)});
}


#pragma region public methods
/**
 * @brief Connects the board to the configured WIFI access point. 
 * 
 * @return true if connection succeeded
 * @return false 
 */
bool Config::Connect_Wifi()
{
  pinMode(LED_BUILTIN, OUTPUT);
  // read configuration from Flash
  if(!this->has_config) this->get_config();
  if(this->ssid.isEmpty())
  {
    Logger.Error("No SSID set. Cannot connect to Wifi");
    delay(500);
    return false;
  }

  if (!WiFi.isConnected())
  {
    digitalWrite(LED_BUILTIN, HIGH); 
    if (this->password[0] == '\0') Logger.Info(F("No WiFi password set. Continue without..."));
    WiFi.begin(this->ssid.c_str(), this->password.c_str());
    Logger.Info("");
    for (int i = 0; i <= 20; i += 1)
    {
      delay(500);
      if (WiFi.isConnected())
      {
        this->wifi_connected = true;
        Logger.Info("");
        Logger.Info_f(F("WiFi connected to %s"), ssid);
        Logger.Info_f(F("IP address: %s"), WiFi.localIP().toString().c_str());
        digitalWrite(LED_BUILTIN, LOW); 
        return true;
      }
      //Serial.print(".");
      //Serial.flush();
    }
    if (!WiFi.isConnected())
    {
      Logger.Error_f(F("Could not connect to WIFI %s..."), ssid);
      return false;
    }
  }
  return true;
}

/**
 * @brief Initialize config structures from EEPROM
 * @param print - true to print the value, false otherwise
 */
void Config::Initialize(bool print)
{
  if(!this->has_config) this->get_config(print);
}

/**
 * @brief Initialize system time from time server.
 * 
 */
void Config::InitializeTime()
{
  Logger.Info("Setting time using SNTP");
  configTime(GMT_OFFSET_SECS, GMT_OFFSET_SECS_DST, NTP_SERVERS);
  time_t now = time(NULL);
  while (now < UNIX_TIME_NOV_13_2017)
  {
    delay(500);
    //Serial.print(".");
    //Serial.flush();
    now = time(nullptr);
  }
  //Serial.println("");
  Logger.Info("Time initialized!");
}

/**
 * @brief Prints the current configuration
 * 
 */
void Config::Print()
{
  //
  // read configuration from Flash
  //
  if(!this->has_config) this->get_config();
  
  Logger.Info_f(F("SSID: %s"), this->ssid.c_str());
  Logger.Info_f(F("SSID Password: %s"), password.length() > 0 ? F("******") : F(""));
  Logger.Info_f(F("Serial Baud Rate: %u"), baud_rate);
  Logger.Info(F("Configured Commands:"));
  for(int idx=0; idx<12; idx++)
    if(Commands[idx]._name_on != "")
      Logger.Info_f(F("     %s"), Commands[idx]._name_on.c_str());
}

/**
 * @brief Starts an access point at the ESP. 
 * Starts a webserver at 192.168.1.4
 * Hosts a website at root (/) to configure the local WiFi.
 * Writes the password to the EEPROM of the ESP
 * Connects to the WiFi, when it is possible with the credentials from the EEPROM. 
 * 
 */
void Config::StartAP()
{
  //
  // read configuration from Flash
  //
  if(!this->has_config) this->get_config();
  
  Logger.Info_f(F("Starting Access Point..."));
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(assid, asecret);
  this->wifi_ap_on = true;

  IPAddress IP = WiFi.softAPIP();
  Logger.Info_f(F("AP IP address: %s"), IP.toString().c_str());
  Logger.Info_f(F("ESP Board MAC Address: %s"), WiFi.macAddress().c_str());
  Logger.Info_f(F("AP SSID: %s"), WiFi.softAPSSID().c_str());
  Logger.Info_f(F("AP Passkey: %s"), asecret);

  server.onNotFound([](AsyncWebServerRequest *request) {
    Logger.Error_f(F("Requested resource not found: %s"), request->url().c_str());
    request->send(404, "text/plain", "Not found");
  });
  server.on("/", HTTP_GET, [=](AsyncWebServerRequest *request) {
    this->values_saved = false;
    request->send_P(200, "text/html", index_html, [=](const String &var){
      return this->processor(var);
    });
  });
  this->server.on("/", HTTP_POST, [=](AsyncWebServerRequest *request) {
    std::regex pattern("([0-9]{1,})_CMD_?(NAME)?_?(ALT)?");
    std::cmatch match;
    int params_amount = request->params();
    for (int i = 0; i < params_amount; i++)
    {
      const AsyncWebParameter *p = request->getParam(i);
      if (strcmp(p->name().c_str(), PARAM_INPUT_ssid) == 0) this->ssid = String(p->value());
      else if (strcmp(p->name().c_str(), PARAM_INPUT_psw) == 0) this->password = String(p->value()); 
      else if (strcmp(p->name().c_str(), "BAUDRATE") == 0) this->baud_rate = (uint32_t)strtol((p->value()).c_str(), NULL, 10);
      else if (std::regex_match(p->name().c_str(), match, pattern))
      {
          // we have a match to a command attribute. 
          if (match[1].matched)
          {
            int idx = std::stoi(match[1].str())-1;
            if(!match[2].matched && !match[3].matched) Commands[idx]._command_on = Command_t::unescape_ctrl_characters(p->value());
            if(!match[2].matched && match[3].matched) Commands[idx]._command_off = Command_t::unescape_ctrl_characters(p->value());
            if(match[2].matched && !match[3].matched) Commands[idx]._name_on = p->value();
            if(match[2].matched && match[3].matched) Commands[idx]._name_off = p->value();
          }
      }
    }
    this->write_values_to_eeprom();
    this->Print();
    request->send_P(200, "text/html", index_html, [=](const String &var){
      return this->processor(var);
    });
  });
  server.begin();
}

/**
 * @brief Stops the access point at the ESP. 
 */
void Config::StopAP()
{
  if(this->wifi_ap_on)
  {
    this->server.end();
    WiFi.softAPdisconnect();
    this->wifi_ap_on = false;
    Logger.Info_f(F("Wifi Access Point shutdown..."));
  }
}
#pragma endregion

#pragma region protected methods
/**
 * @brief Reads the configuration from EEPROM
 * @param print - true to print the value, false otherwise
 * 
 */
void Config::get_config(bool print)
{  
  this->read_values_from_eeprom();
  this->has_config = true;
  if(print) this->Print();
}
#pragma endregion

#pragma region private methods
/**
 * @brief Implements the 404 Handler for the web server
 * 
 * @param request - The request
 */
void Config::not_found(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

/**
 * @brief Returns the actual values corresponding to various tokens in the page. 
 * 
 * @param var - the token to replace
 * @return String - the value for the token.
 */
String Config::processor(const String &var)
{
  std::regex pattern("([0-9]{1,})_CMD_?(NAME)?_?(ALT)?");
  std::cmatch match;
  if (var == "SUCCESSFULLY_CONNECTED" && this->wifi_connected == true) return F("<p style=\"color:green;\">Successfully connected to WiFi!</p>");
  if (var == "SUCCESSFULLY_CONNECTED" && this->wifi_connected == false) return F("<p style=\"color:red;\">Not connected to WiFi!</p>");
  if (var == "PLEASE_RESTART" && this->values_saved == true) return F("<p>Settings saved! Please restart the controller.</p>");
  if (var == "PLEASE_RESTART" && this->values_saved == false) return F("");
  if (var == "SSID") return this->ssid;
  if (var == "PWD") return this->password;
  if (var == "BAUDRATE") return String(this->baud_rate);
  if (std::regex_match(var.c_str(), match, pattern))
  { 
    if (match[1].matched)
    {
      int idx = std::stoi(match[1].str())-1;
      if(!match[2].matched && !match[3].matched) return Command_t::escape_ctrl_characters(Commands[idx]._command_on);
      if(!match[2].matched && match[3].matched) return Command_t::escape_ctrl_characters(Commands[idx]._command_off);
      if(match[2].matched && !match[3].matched) return Commands[idx]._name_on;
      if(match[2].matched && match[3].matched) return Commands[idx]._name_off;
    }
  }
  Logger.Info_f(F("Unknonw token: %s"), var.c_str());
  return String();
}

/**
 * @brief Reads string from EEPROM
 * 
 * @param addrOffset - offset at which to read. 
 * @param pos - pointer to interger holding the position of the next value after read.
 * @return String
 */
String Config::read_string_from_eeprom(int addrOffset, int *pos)
{
  int i = 0;
  byte b2 = EEPROM.read(addrOffset++);
  byte b1 = EEPROM.read(addrOffset++);
  uint16_t len = 0;
  len |= b2;
  len <<= 8;
  len |= b1;
 
  if(len == 65535) len = 0;
    // EEPROM might be initialized with 0xF instead of 0x0. 
    // this will help in those cases. 

  char data[len + 1];
  for (i = 0; i < len; i++) data[i] = EEPROM.read(addrOffset + i);
  data[i] = '\0';
  *pos = addrOffset + i;

  return String(data);
}

/**
 * @brief Reads configuration values from EEPROM
 * 
 */
void Config::read_values_from_eeprom()
{
  // the first two eeprom bytes simply contain the string OK. That is to indicate that the eeprom partition has been written at least once. 
  // this is followed by a byte containin the length of the serialized JSON config string followed by hte string itself.  
  int pos = 0;
  if(EEPROM.begin(EEPROM_SIZE)) Logger.Info(F("EEPROM initialized."));
  else 
  {
    Logger.Error(F("Critital - Unable to initialize EEPROM."));
    return;
  }
  if(read_string_from_eeprom(pos, &pos) == "OK")
  {
    Logger.Info(F("Found valid EEPROM block"));
    String s = read_string_from_eeprom(pos, &pos);
    DynamicJsonDocument d = DynamicJsonDocument(EEPROM_SIZE);
    deserializeJson(d, s);
    this->ssid = String(d["ssid"].as<String>());
    this->password = String(d["pwd"].as<String>());
    this->baud_rate = d["speed"];
    int idx=0;
    for (JsonObject command : d["commands"].as<JsonArray>()) 
    {
      this->Commands[idx]._command_on = String(command["c"].as<String>());
      this->Commands[idx]._name_on = String(command["cn"].as<String>());
      this->Commands[idx]._command_off = String(command["ca"].as<String>());
      this->Commands[idx]._name_off = String(command["cna"].as<String>());
      idx++;
    }
  }
  EEPROM.end();
}

/**
 * @brief Writes a string to EEPROM
 * 
 * @param addrOffset - offset at which to write
 * @param str_to_write - string to write
 * @return int the position in the Flash where the next object can be written
 */
int Config::write_string_to_eeprom(int addrOffset, const String &str_to_write)
{
  int i = 0;
  uint16_t len = str_to_write.length();
  byte b1 = len & 0x000000ff;
  byte b2 = (len & 0x0000ff00) >> 8;
  EEPROM.write(addrOffset++, b2);
  EEPROM.write(addrOffset++, b1);
  if (len > 0)
  {
    for (i = 0; i < len; i++)
    {
      EEPROM.write(addrOffset + i, str_to_write[i]);
    }
  }
  return addrOffset + i;
}

/**
 * @brief Write configuration values to EEPROM
 * 
 */
void Config::write_values_to_eeprom()
{
  // the first two eeprom bytes simply contain the string OK. That is to indicate that the eeprom partition has been written at least once. 
  // this is followed by a byte containin the length of the serialized JSON config string followed by hte string itself.   
  if(EEPROM.begin(EEPROM_SIZE)) Logger.Info(F("EEPROM initialized."));
  else 
  {
    Logger.Error(F("Critital - Unable to initialize EEPROM."));
    return;
  }
  int pos = write_string_to_eeprom(0, "OK");

  DynamicJsonDocument config = DynamicJsonDocument(EEPROM_SIZE);
  config["ssid"] = this->ssid;
  config["pwd"] = this->password;
  config["speed"] = this->baud_rate;
  JsonArray commands = config["commands"].to<JsonArray>();
  for(int i=0; i<12; i++)
  {
    JsonObject command = commands.createNestedObject();
    command["c"] = this->Commands[i]._command_on;
    command["cn"] = this->Commands[i]._name_on;
    command["ca"] = this->Commands[i]._command_off;
    command["cna"] = this->Commands[i]._name_off;
  }
  int size = measureJson(config) + 1;
  char *buf = (char *)malloc(size*sizeof(char));
  serializeJson(config, buf, size);
  pos = write_string_to_eeprom(pos, buf);
  free(buf);

  if(!EEPROM.commit()) Logger.Error(F("Critical - Unable to commit EEPROM changes."));
  EEPROM.end();

  Logger.Info(F("New value saved in flash memory"));
  values_saved = true;  
}
#pragma endregion

