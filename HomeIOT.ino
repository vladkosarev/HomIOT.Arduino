/* HomIOT Arduino firmware
 Enables ESP8266 and sends PIN information on state change
 ESP8266 communicates with the HomIOT server
 Created Dec 13/2014 by Vlad Kosarev
 Source: https://github.com/vladkosarev/HomIOT.Arduino
*/

#include <SoftwareSerial.h>

#define SSID "" // your wifi ssid
#define PASS "" // your wifi password
#define DST_IP "192.168.0.21" // HomIOT Server IP
#define DST_PORT 9999 // HomIOT Server port
#define PIN_OFFSET 2 // home many pins to ignore, by default ignore first two (used for soft serial)
#define PIN_MAX 12 // up to what pin to watch
#define ERROR_LED 13 // LED to enable on startup error

SoftwareSerial dbgSerial(10, 11); // Debug Soft Serial RX, TX
int currentInputState;
int lastInputState[PIN_MAX + 1];

void setup()
{
	pinMode(ERROR_LED, OUTPUT);
	Serial.begin(57600); // This works reliably with 3.3v 8Mhz arduino
	Serial.setTimeout(5000);
	dbgSerial.begin(9600);
	dbgSerial.println("HomIOT Started...");

	for (int i = PIN_OFFSET; i <= PIN_MAX; i++)
	{
  		pinMode(i, OUTPUT);
  		lastInputState[i] = -1;
	}

	if (!detectESP8266())
	{
		fail();
	}

	delay(500);

	boolean connected = false;
	for (int i = 0; i < 5; i++)
	{
		if (connectToWiFi())
		{
			connected = true;
			break;
		}
	}
	if (!connected){ fail(); }

	delay(500);
	printIP();	
	Serial.println("AT+CIPMUX=0"); //set the single connection mode
}

void loop()
{
        checkPins();
}

void checkPins()
{
        bool dataToSend = false;
        String data = "{";
  	for (int i = PIN_OFFSET; i <= PIN_MAX; i++)
	{
		currentInputState = digitalRead(i);
		if (currentInputState != lastInputState[i]) {
                        if (data.length() > 1) data += ","; //if we already have data
                        data += "\"P";
                        data += i;
                        data += "\":";
                        data += currentInputState;
                        dataToSend = true;
                        lastInputState[i] = currentInputState;
		}
	}
        data += "}\r\n";
        if(dataToSend) {
          	sendData(data);
        }
}

boolean sendData(String data)
{
	String cmd = "AT+CIPSTART=\"TCP\",\"";
	cmd += DST_IP;
	cmd += "\",";
        cmd += DST_PORT;
	Serial.println(cmd);
	dbgSerial.println(cmd);
	if (Serial.find("ERROR")) {
  		dbgSerial.println("ERROR Sending data.");
                return false;
        }
	cmd = "POST / HTTP/1.0\r\nContent-Type:application/json \r\nConnection: close\r\n";
	cmd += "Content-Length: ";
	cmd += data.length();
	cmd += "\r\n\r\n";
	cmd += data;
	Serial.print("AT+CIPSEND=");
	Serial.println(cmd.length());
        delay(500);
	if (Serial.find(">"))
	{
		dbgSerial.print(">");
	}
	else
	{
		Serial.println("AT+CIPCLOSE");
		dbgSerial.println("Connection timed out.");
		delay(500);
		return false;
	}
	Serial.print(cmd);
	/*delay(500);
	while (Serial.available())
	{
		char c = Serial.read();
		dbgSerial.write(c);
		if (c == '\r') dbgSerial.print('\n');
	}*/
	dbgSerial.println("Send Successful.");
        delay(1000);
	return true;
}

boolean connectToWiFi()
{
	Serial.println("AT+CWMODE=1");
	String cmd = "AT+CWJAP=\"";
	cmd += SSID;
	cmd += "\",\"";
	cmd += PASS;
	cmd += "\"";
	dbgSerial.println(cmd);
	Serial.println(cmd);
	delay(3000);
	if (Serial.find("OK"))
	{
		dbgSerial.println("Connected to WiFi.");
		return true;
	}
	else
	{
		dbgSerial.println("Failed to connect to WiFi.");
		return false;
	}
}

boolean detectESP8266()
{
	Serial.println("AT+RST");
	delay(2000);
	if (Serial.find("OK"))
	{
		dbgSerial.println("ESP8266 found...");
		return true;
	}
	else
	{
		dbgSerial.println("ESP8266 NOT found...");
		return false;
	}
}

void printIP()
{
	Serial.println("AT+CIFSR");
	dbgSerial.println("ip address:");
	while (Serial.available())
		dbgSerial.write(Serial.read());
}

void fail()
{
	digitalWrite(ERROR_LED, HIGH);
	while (1);
}
