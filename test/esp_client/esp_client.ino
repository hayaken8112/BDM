
//https://github.com/Links2004/arduinoWebSockets/issues/33
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets
//myip 27

//IP
const char* Server_ip("192.168.11.31");
int port = 81;

//ssid, pass
const char* ssid = "Buffalo-G-95F5";
const char* password = "bnhcsau8trtk3";

WebSocketsClient webSocket;


void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected!\n");
            break;

        case WStype_CONNECTED:
            {
                Serial.printf("[WSc] Connected to url: %s\n",  payload);
            }
            break;

        case WStype_TEXT:
           Serial.printf("[WSc]Recv: %s\n", payload);

            // send data to back to Server
            //webSocket.sendTXT(payload, lenght);
            break;

        case WStype_BIN:
            Serial.printf("[WSc] get binary lenght: %u\n", lenght);
            hexdump(payload, lenght);

            // echo data back to Server
            webSocket.sendBIN(payload, lenght);
            break;

        case WStype_ERROR:
            Serial.printf("We received an error \n");
            break;

        default:
            break;
    }

}


void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.print("\n\nStart\n");

    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(500);
        }
    Serial.println();
    Serial.printf("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    //serverip
    webSocket.begin(Server_ip, port);
    webSocket.onEvent(webSocketEvent);
    Serial.println("Client started\n");

}


void loop() {
    webSocket.loop();
    webSocket.sendTXT("hello"); //send
}