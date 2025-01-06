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
            <tr><th>Name</th><th>Command</th><th>Alternate Name</th><th>Alternate command</th></tr>
            <tr><td><input type="text" maxlength="24" size="24" id="1_CMD_NAME" name="1_CMD_NAME" value="%1_CMD_NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="1_CMD" name="1_CMD" value="%1_CMD%"></td>
                <td><input type="text" maxlength="24" size="24" id="1_CMD_NAME_ALT" name="1_CMD_NAME_ALT" value="%1_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="1_CMD_ALT" name="1_CMD_ALT" value="%1_CMD_ALT%"></td></tr>
            <tr><td><input type="text" maxlength="24" size="24" id="2_CMD_NAME" name="2_CMD_NAME" value="%2_CMD_NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="2_CMD" name="2_CMD" value="%2_CMD%"></td>
                <td><input type="text" maxlength="24" size="24" id="2_CMD_NAME_ALT" name="2_CMD_NAME_ALT" value="%2_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="2_CMD_ALT" name="2_CMD_ALT" value="%2_CMD_ALT%"></td></tr>
            <tr><td><input type="text" maxlength="24" size="24" id="3_CMD_NAME" name="3_CMD_NAME" value="%3_CMD_NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="3_CMD" name="3_CMD" value="%3_CMD%"></td>
                <td><input type="text" maxlength="24" size="24" id="3_CMD_NAME_ALT" name="3_CMD_NAME_ALT" value="%3_CMD_NAME_ALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="3_CMD_ALT" name="3_CMD_ALT" value="%3_CMD_ALT%"></td></tr>
            <tr><td><input type="text" maxlength="24" size="24" id="CMD4NAME" name="CMD4NAME" value="%CMD4NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD4" name="CMD4" value="%CMD4%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD4NAMEALT" name="CMD4NAMEALT" value="%CMD4NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD4ALT" name="CMD4ALT" value="%CMD4ALT%"></td></tr>  
            <tr><td><input type="text" maxlength="24" size="24" id="CMD5NAME" name="CMD5NAME" value="%CMD5NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD5" name="CMD5" value="%CMD5%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD5NAMEALT" name="CMD5NAMEALT" value="%CMD5NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD5ALT" name="CMD5ALT" value="%CMD5ALT%"></td></tr> 
            <tr><td><input type="text" maxlength="24" size="24" id="CMD6NAME" name="CMD6NAME" value="%CMD6NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD6" name="CMD6" value="%CMD6%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD6NAMEALT" name="CMD6NAMEALT" value="%CMD6NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD6ALT" name="CMD6ALT" value="%CMD6ALT%"></td></tr> 
            <tr><td><input type="text" maxlength="24" size="24" id="CMD7NAME" name="CMD7NAME" value="%CMD7NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD7" name="CMD7" value="%CMD7%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD7NAMEALT" name="CMD7NAMEALT" value="%CMD7NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD7ALT" name="CMD7ALT" value="%CMD7ALT%"></td></tr> 
            <tr><td><input type="text" maxlength="24" size="24" id="CMD8NAME" name="CMD8NAME" value="%CMD8NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD8" name="CMD8" value="%CMD8%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD8NAMEALT" name="CMD8NAMEALT" value="%CMD8NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD8ALT" name="CMD8ALT" value="%CMD8ALT%"></td></tr> 
            <tr><td><input type="text" maxlength="24" size="24" id="CMD9NAME" name="CMD9NAME" value="%CMD9NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD9" name="CMD9" value="%CMD9%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD9NAMEALT" name="CMD4NAMEALT" value="%CMD9NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD9ALT" name="CMD4ALT" value="%CMD9ALT%"></td></tr>            
            <tr><td><input type="text" maxlength="24" size="24" id="CMD10NAME" name="CMD10NAME" value="%CMD10NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD10" name="CMD10" value="%CMD10%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD10NAMEALT" name="CMD10NAMEALT" value="%CMD10NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD10ALT" name="CMD10ALT" value="%CMD10ALT%"></td></tr>      
            <tr><td><input type="text" maxlength="24" size="24" id="CMD11NAME" name="CMD11NAME" value="%CMD11NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD11" name="CMD11" value="%CMD11%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD11NAMEALT" name="CMD11NAMEALT" value="%CMD11NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD11ALT" name="CMD11ALT" value="%CMD11ALT%"></td></tr>      
            <tr><td><input type="text" maxlength="24" size="24" id="CMD12NAME" name="CMD12NAME" value="%CMD12NAME%"></td> 
                <td><input type="text" maxlength="48" size="64" id="CMD12" name="CMD12" value="%CMD12%"></td>
                <td><input type="text" maxlength="24" size="24" id="CMD12NAMEALT" name="CMD12NAMEALT" value="%CMD12NAMEALT%"></td>
                <td><input type="text" maxlength="48" size="64" id="CMD12ALT" name="CMD12ALT" value="%CMD12ALT%"></td></tr>              
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
