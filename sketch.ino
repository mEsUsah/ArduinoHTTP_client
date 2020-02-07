/**********************************************************************************************
    --- Arduino Ehternet HTTP webclient ---
    Written by Stanley Skarshaug, 07.02.2020 - Norway.
    Free to use - if credit is given to author.

    Simple web client that sends a PUT request to webserver for logging purpose.
    Prints the HTTP request to the serial monitor for debuging and
    prints the first line of the HTTP respone to the serial monitor for debuging.
    
**********************************************************************************************/

#include<SPI.h>
#include<Ethernet.h>


const int httpLEDPin		= 4;				        // LED indicator for http activity. Both as a server and a client
const int errorLEDPin		= 5;        		        // LED indicator for any system error 

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };	// Custom MAC-address. Can be pretty much anything...
IPAddress ip(10,22,22,60);								// Static IP Address
IPAddress myDns(8,8,8,8);								// DNS Server - dns.google 
IPAddress myGateway(10,22,22,1);

EthernetClient client;         							// initialize the Ethernet client object - Will be used to transmit data to remote server
char webServerURL_IP[] ="10.22.22.50"; 					// Server URL or IP. 
String hostName = "api.skarshaugs.no";					// VirtualHost on server
String apiKey = "s3cr37";								// Key to access the API. Must be provided in order to use API

String readString;
bool firstLine = true;

void setup(){
	pinMode(httpLEDPin, OUTPUT);
	pinMode(errorLEDPin, OUTPUT);

	Serial.begin(9600);
	Serial.println("OK - Serial communication is established ");

	Ethernet.begin(mac, ip, myDns, myGateway);

	if(Ethernet.localIP()){
		Serial.print("Arduino IP address: ");
		Serial.println(Ethernet.localIP());
		Serial.println("OK - Ethernet system is ready...");
	} else {
		Serial.println("ERROR! - Failed to configure Ethernet using static IP");
		digitalWrite(errorLEDPin, HIGH); 				// turn ON Error LED indicator of DHCP setup failed.
		while (true); 									// Stop program.
	}

	Serial.println("OK - All systems active");
	Serial.println();
}

void loop() {
	httpRequest("POST /tempLog?apikey="+apiKey+"temp=29&hygr=55 HTTP/1.1");
    delay(10000);
}

/***********************************************************************************************************************/
/*                    Send data to the web server with HTTP a request                                                  */
/***********************************************************************************************************************/
void httpRequest(String request) {
	if (client.connect(webServerURL_IP,80)) {  		    //connect the "Arduino" as client to the web server using web socket
		digitalWrite(httpLEDPin, HIGH);     		    //Turn ON http communication LED
		digitalWrite(errorLEDPin, LOW);

		// Print GET request to serial monitor for debug.     
		Serial.println("Connected to host");
		Serial.println("sending data to web server: " + String(webServerURL_IP));      
		Serial.println(request);
		Serial.println("Host: " + hostName);
		Serial.println("Connection: close"); 		
		Serial.println(); 

		//print GET request to client object.
		client.println(request);					    // print the actual request
		client.println("Host: api.skarshaugs.no");	    // specify the virtual host in the HTTP head as a variable
		client.println("Connection: close"); 		    // telling the server that we are over transmitting the message
		client.println();                    		    // empty line. This marks the end of the HTTP request.

		// Print to the console the response from the server
		httpResponse();

	} else {
		Serial.println("--> connection failed\n");
		digitalWrite(errorLEDPin, HIGH);
	}

	if (client.connected()) { 
	  	client.stop();                         	        // close communication socket 
	}

	digitalWrite(httpLEDPin, LOW);           	        // Turn OFF the http communication LED
}

void httpResponse() {
	long timestamp=millis();
	bool currentLineIsBlank = true;
    if(client.connected()){
        digitalWrite(httpLEDPin, HIGH);     		    //Turn ON http communication LED
    }
	while (client.connected()) {
        if (client.available()) {
			char c = client.read();

			//Serial.write(c);

			if(readString.length()<100 && firstLine){
				readString += c;
				
				// This API only cares about the first line of the HTTP request.
				if(c == '\n'){
					firstLine = false;
					Serial.print(readString);

					// Reset the readString to be ready for a new connection.
					readString = "";
				}
			}

			//Wait for the response header to finish. It will end with a empty line containing a newline character.
			//Then close the connection. In this application we don't care about the resonse body.
			if (c == '\n' && currentLineIsBlank) {
				break;
			}
			if (c == '\n') {
				currentLineIsBlank = true;
			}
			else if (c != '\r') {
				currentLineIsBlank = false;
			}
		}
	}
	Serial.println("disconnecting.");
	client.stop();                                      // closing connection to server
    digitalWrite(httpLEDPin, LOW);           	        // Turn OFF the http communication LED
	Serial.println("*******************************************************************************************");
	Serial.println();
	firstLine = true;
}