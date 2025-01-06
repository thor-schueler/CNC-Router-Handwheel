// Copyright (c) Avanade. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef CONFIG_PAGE_H_
#define CONFIG_PAGE_H_

#include <WiFi.h>
#include <WiFiAP.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include "html.h"
#include "../wheel/wheel.h"

// Utility macros and time defines
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define UNIX_TIME_NOV_13_2017 1510592825

#define GST_TIME_ZONE -6
#define GST_TIME_ZONE_DAYLIGHT_SAVINGS_DIFF   1
#define GMT_OFFSET_SECS (GST_TIME_ZONE * 3600)
#define GMT_OFFSET_SECS_DST (GST_TIME_ZONE_DAYLIGHT_SAVINGS_DIFF * 3600)

// enables EEPROM usage
#define EEPROM_SIZE 8192

//Access point configuration
#define ACCESS_POINT_NAME "cnc_hand_wheel"
#define ACCESS_POINT_PWD "Infusi0n"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif


/**
 * @brief This class manages settings serializing and deserialization from EEPROM, the configuration web server, 
 * Connection to Wifi and time management. 
 * 
 */
class Config
{
  public:

    /**
     * @brief Construct a new Config object
     * 
     */
    Config();
    
    /**
     * @brief Connects the board to the configured WIFI access point. 
     * 
     * @return true if connection succeeded
     * @return false 
     */
    bool Connect_Wifi();

    /**
    * @brief Initialize config structures from EEPROM
    * @param print - true to printout the config values
    */
    void Initialize(bool print=true);

    /**
     * @brief Initialize system time from time server.
     * 
     */
    void InitializeTime();

    /**
     * @brief Prints the current configuration
     * 
     */    
    void Print();

    /**
     * @brief Starts an access point at the ESP. 
     * Starts a webserver at 192.168.1.4
     * Hosts a website at root (/) to configure the local WiFi.
     * Writes the password to the EEPROM of the ESP
     * Connects to the WiFi, when it is possible with the credentials from the EEPROM. 
     * 
     */
    void StartAP();
    
    /**
     * @brief Stops the access point at the ESP. 
     */
    void StopAP();

    /**
     * @brief The WIFI SSID
     * 
     */
    String ssid = "";

    uint32_t baud_rate = 115200;
    std::unordered_map<uint8_t, Command_t> Commands;

  protected:
    /**
     * @brief Reads the configuration from EEPROM
     * @param print - true to print the value, false otherwise
     */
    void get_config(bool print=true);
    
    /**
     * @brief The WIFI password
     * 
     */
    String password = "";

  private:
    /**
     * @brief Implements the 404 Handler for the web server
     * 
     * @param request - The request
     */
    static void not_found(AsyncWebServerRequest *request);
    
    /**
     * @brief Returns the actual values corresponding to various tokens in the page. 
     * 
     * @param var - the token to replace
     * @return String - the value for the token.
     */    
    String processor(const String &var);
    
    /**
     * @brief Reads string from EEPROM
     * 
     * @param addrOffset - offset at which to read. 
     * @param pos - pointer to interger holding the position of the next value after read.
     * @return String
     */
    static String read_string_from_eeprom(int addrOffset, int *pos);
    
    /**
     * @brief Reads configuration values from EEPROM
     * 
     */
    void read_values_from_eeprom();
    
    /**
     * @brief Writes a string to EEPROM
     * 
     * @param addrOffset - offset at which to write
     * @param str_to_write - string to write
     * @return int the position in the Flash where the next object can be written
     */    
    static int write_string_to_eeprom(int addrOffset, const String &str_to_write);
    
    /**
     * @brief Write configuration values to EEPROM
     * 
     */    
    void write_values_to_eeprom();

    bool has_config = false;
    bool values_saved = false;
    bool wifi_connected = false;
    bool wifi_ap_on = false;
    static AsyncWebServer server;
};

#endif /* CONFIG_PAGE_H_ */
