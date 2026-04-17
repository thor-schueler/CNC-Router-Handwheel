/**
 * @brief HTML web page to configure the WiFi of the robot.
 * 
 */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>CNC Wheel Configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="author" content="Thor Schueler">
</head>
<body>
  <article>
    <header>
      <h1>CNC Wheel Configuration</h1>
    </header>
    <section>
      <form action="/" method="post">
        <h3>WIFI Configuration</h3>
        <label for="SSID">Wlan Name (SSID):</label><br><input maxlength="32" size="32" type="text" id="SSID" name="SSID" value="%SSID%"><br>
        <label for="psw">Password:</label><br><input type="password" id="psw" maxlength="32" size="32" name="psw" value="%PWD%"><br><br>
        <hr/>
        <h3>Command Configuration</h3>
        <table border="0"> 
            <tr><th>Name</th><th>Command</th><th>Alternate Name</th><th>Alternate command</th><th>Zero X</th><th>Zero Y</th><th>Zero Z</th></tr>
            <tr>
                <td><input type="text" maxlength="24" size=16" id="1_CMD_NAME" name="1_CMD_NAME" value="%1_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="1_CMD" name="1_CMD" value="%1_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="1_CMD_NAME_ALT" name="1_CMD_NAME_ALT" value="%1_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="1_CMD_ALT" name="1_CMD_ALT" value="%1_CMD_ALT%"></td>
                <td><input type="checkbox" id="1_CMD_0X" name="1_CMD_0X" %1_CMD_0X%></td>
                <td><input type="checkbox" id="1_CMD_0Y" name="1_CMD_0Y" %1_CMD_0Y%></td>
                <td><input type="checkbox" id="1_CMD_0Z" name="1_CMD_0Z" %1_CMD_0Z%></td>
            </tr>
            <tr>
                <td><input type="text" maxlength="24" size="16" id="2_CMD_NAME" name="2_CMD_NAME" value="%2_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="2_CMD" name="2_CMD" value="%2_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="2_CMD_NAME_ALT" name="2_CMD_NAME_ALT" value="%2_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="2_CMD_ALT" name="2_CMD_ALT" value="%2_CMD_ALT%"></td>
                <td><input type="checkbox" id="2_CMD_0X" name="2_CMD_0X" %2_CMD_0X%></td>
                <td><input type="checkbox" id="2_CMD_0Y" name="2_CMD_0Y" %2_CMD_0Y%></td>
                <td><input type="checkbox" id="2_CMD_0Z" name="2_CMD_0Z" %2_CMD_0Z%></td>
            </tr>
            <tr>
                <td><input type="text" maxlength="24" size="16" id="3_CMD_NAME" name="3_CMD_NAME" value="%3_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="3_CMD" name="3_CMD" value="%3_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="3_CMD_NAME_ALT" name="3_CMD_NAME_ALT" value="%3_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="3_CMD_ALT" name="3_CMD_ALT" value="%3_CMD_ALT%"></td>
                <td><input type="checkbox" id="3_CMD_0X" name="3_CMD_0X" %3_CMD_0X%></td>
                <td><input type="checkbox" id="3_CMD_0Y" name="3_CMD_0Y" %3_CMD_0Y%></td>
                <td><input type="checkbox" id="3_CMD_0Z" name="3_CMD_0Z" %3_CMD_0Z%></td>
            </tr>
            <tr>
                <td><input type="text" maxlength="24" size="16" id="4_CMD_NAME" name="4_CMD_NAME" value="%4_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="4_CMD" name="4_CMD" value="%4_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="4_CMD_NAME_ALT" name="4_CMD_NAME_ALT" value="%4_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="4_CMD_ALT" name="4_CMD_ALT" value="%4_CMD_ALT%"></td>
                <td><input type="checkbox" id="4_CMD_0X" name="4_CMD_0X" %4_CMD_0X%></td>
                <td><input type="checkbox" id="4_CMD_0Y" name="4_CMD_0Y" %4_CMD_0Y%></td>
                <td><input type="checkbox" id="4_CMD_0Z" name="4_CMD_0Z" %4_CMD_0Z%></td>
            </tr>
            <tr>
                <td><input type="text" maxlength="24" size="16" id="5_CMD_NAME" name="5_CMD_NAME" value="%5_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="5_CMD" name="5_CMD" value="%5_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="5_CMD_NAME_ALT" name="5_CMD_NAME_ALT" value="%5_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="5_CMD_ALT" name="5_CMD_ALT" value="%5_CMD_ALT%"></td>
                <td><input type="checkbox" id="5_CMD_0X" name="5_CMD_0X" %5_CMD_0X%></td>
                <td><input type="checkbox" id="5_CMD_0Y" name="5_CMD_0Y" %5_CMD_0Y%></td>
                <td><input type="checkbox" id="5_CMD_0Z" name="5_CMD_0Z" %5_CMD_0Z%></td>
            </tr>
            <tr>
                <td><input type="text" maxlength="24" size="16" id="6_CMD_NAME" name="6_CMD_NAME" value="%6_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="6_CMD" name="6_CMD" value="%6_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="6_CMD_NAME_ALT" name="6_CMD_NAME_ALT" value="%6_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="6_CMD_ALT" name="6_CMD_ALT" value="%6_CMD_ALT%"></td>
                <td><input type="checkbox" id="6_CMD_0X" name="6_CMD_0X" %6_CMD_0X%></td>
                <td><input type="checkbox" id="6_CMD_0Y" name="6_CMD_0Y" %6_CMD_0Y%></td>
                <td><input type="checkbox" id="6_CMD_0Z" name="6_CMD_0Z" %6_CMD_0Z%></td>
            </tr> 
            <tr>
                <td><input type="text" maxlength="24" size="16" id="7_CMD_NAME" name="7_CMD_NAME" value="%7_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="7_CMD" name="7_CMD" value="%7_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="7_CMD_NAME_ALT" name="7_CMD_NAME_ALT" value="%7_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="7_CMD_ALT" name="7_CMD_ALT" value="%7_CMD_ALT%"></td>
                <td><input type="checkbox" id="7_CMD_0X" name="7_CMD_0X" %7_CMD_0X%></td>
                <td><input type="checkbox" id="7_CMD_0Y" name="7_CMD_0Y" %7_CMD_0Y%></td>
                <td><input type="checkbox" id="7_CMD_0Z" name="7_CMD_0Z" %7_CMD_0Z%></td>
            </tr> 
            <tr>
                <td><input type="text" maxlength="24" size="16" id="8_CMD_NAME" name="8_CMD_NAME" value="%8_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="8_CMD" name="8_CMD" value="%8_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="8_CMD_NAME_ALT" name="8_CMD_NAME_ALT" value="%8_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="8_CMD_ALT" name="8_CMD_ALT" value="%8_CMD_ALT%"></td>
                <td><input type="checkbox" id="8_CMD_0X" name="8_CMD_0X" %8_CMD_0X%></td>
                <td><input type="checkbox" id="8_CMD_0Y" name="8_CMD_0Y" %8_CMD_0Y%></td>
                <td><input type="checkbox" id="8_CMD_0Z" name="8_CMD_0Z" %8_CMD_0Z%></td>
            </tr>
            <tr>
                <td><input type="text" maxlength="24" size="16" id="9_CMD_NAME" name="9_CMD_NAME" value="%9_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="9_CMD" name="9_CMD" value="%9_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="9_CMD_NAME_ALT" name="9_CMD_NAME_ALT" value="%9_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="9_CMD_ALT" name="9_CMD_ALT" value="%9_CMD_ALT%"></td>
                <td><input type="checkbox" id="9_CMD_0X" name="9_CMD_0X" %9_CMD_0X%></td>
                <td><input type="checkbox" id="9_CMD_0Y" name="9_CMD_0Y" %9_CMD_0Y%></td>
                <td><input type="checkbox" id="9_CMD_0Z" name="9_CMD_0Z" %9_CMD_0Z%></td>
            </tr>            
            <tr>
                <td><input type="text" maxlength="24" size="16" id="10_CMD_NAME" name="10_CMD_NAME" value="%10_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="10_CMD" name="10_CMD" value="%10_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="10_CMD_NAME_ALT" name="10_CMD_NAME_ALT" value="%10_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="10_CMD_ALT" name="10_CMD_ALT" value="%10_CMD_ALT%"></td>
                <td><input type="checkbox" id="10_CMD_0X" name="10_CMD_0X" %10_CMD_0X%></td>
                <td><input type="checkbox" id="10_CMD_0Y" name="10_CMD_0Y" %10_CMD_0Y%></td>
                <td><input type="checkbox" id="10_CMD_0Z" name="10_CMD_0Z" %10_CMD_0Z%></td>
            </tr>      
            <tr>
                <td><input type="text" maxlength="24" size="16" id="11_CMD_NAME" name="11_CMD_NAME" value="%11_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="11_CMD" name="11_CMD" value="%11_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="11_CMD_NAME_ALT" name="11_CMD_NAME_ALT" value="%11_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="11_CMD_ALT" name="11_CMD_ALT" value="%11_CMD_ALT%"></td>
                <td><input type="checkbox" id="11_CMD_0X" name="11_CMD_0X" %11_CMD_0X%></td>
                <td><input type="checkbox" id="11_CMD_0Y" name="11_CMD_0Y" %11_CMD_0Y%></td>
                <td><input type="checkbox" id="11_CMD_0Z" name="11_CMD_0Z" %11_CMD_0Z%></td>
            </tr>      
            <tr>
                <td><input type="text" maxlength="24" size="16" id="12_CMD_NAME" name="12_CMD_NAME" value="%12_CMD_NAME%"></td> 
                <td><input type="text" maxlength="128" size="48" id="12_CMD" name="12_CMD" value="%12_CMD%"></td>
                <td><input type="text" maxlength="24" size="16" id="12_CMD_NAME_ALT" name="12_CMD_NAME_ALT" value="%12_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="128" size="48" id="12_CMD_ALT" name="12_CMD_ALT" value="%12_CMD_ALT%"></td>
                <td><input type="checkbox" id="12_CMD_0X" name="12_CMD_0X" %12_CMD_0X%></td>
                <td><input type="checkbox" id="12_CMD_0Y" name="12_CMD_0Y" %12_CMD_0Y%></td>
                <td><input type="checkbox" id="12_CMD_0Z" name="12_CMD_0Z" %12_CMD_0Z%></td>
            </tr>              
        </table>
        <hr/>        
        <h3>Connection Settings</h3>
        <label for="BAUDRATE">Serial Connection Baudrate:</label><br><input type="number" maxlength="16" size="16" id="BAUDRATE" name="BAUDRATE" value="%BAUDRATE%"><br>
        <hr/>
        <input type="submit" value="Save Configuration">
      </form> 
      %PLEASE_RESTART%
      %SUCCESSFULLY_CONNECTED%
    </section>
    <footer>
      <p style="color:Gainsboro; font-size: x-small;">copyright 2018-2022 - Avanade.</p>
    </footer>
</body>
</html>)rawliteral";
