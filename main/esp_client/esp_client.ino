#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets
#include <Wire.h>             //I2Cライブラリ
#include <SparkFunLSM9DS1.h>  //https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
#include <Adafruit_NeoPixel.h>

// ----------------- WiFi settings -------------------
//IP
const char *Server_ip("192.168.11.31");
int port = 81;

//ssid, pass
const char *ssid = "Buffalo-G-95F5";
const char *password = "bnhcsau8trtk3";

WebSocketsClient webSocket;

// ------------------- LED settings -----------------
#define MAX_VAL 20 // 0 to 255 for brightness
#define DELAY_TIME 50
#define DELAY_TIME2 20
#define ADAddr 0x48

Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, 0, NEO_GRB + NEO_KHZ800);

uint32_t color1 = strip.Color(MAX_VAL, 0, 0);
uint32_t color2 = strip.Color(0, MAX_VAL, 0);

// ------------------- sensor settings ---------------
//-------------------------------------------------------------------------
float gxVal = 0;  //ジャイロｘ軸用データーレジスタ
float gyVal = 0;  //ジャイロｙ軸用データーレジスタ
float gzVal = 0;  //ジャイロｚ軸用データーレジスタ
float axVal = 0;  //Axis ｘ用データーレジスタ
float ayVal = 0;  //Axis ｙ用データーレジスタ
float azVal = 0;  //Axis ｚ用データーレジスタ
float mxVal = 0;  //Mag x 用データーレジスタ
float myVal = 0;  //Mag ｙ 用データーレジスタ
float mzVal = 0;  //Mag x 用データーレジスタ
float hedVal = 0; //Hedding 用データーレジスタ

float offset_mx = 1400;
float offset_my = -1800;

//------------------------------------------------------------------------
LSM9DS1 imu;

uint8_t current_index = 0;

// ---------------------- data settings ----------------
#define NUM_OF_DATA 4

uint8_t *packet;
float send_data[NUM_OF_DATA];
float heading = 0;
float roll = 0;
float pitch = 0;

float *rec_data;

size_t packet_size = NUM_OF_DATA * sizeof(float);

#define LSM9DS1_M 0x1E  // SPIアドレス設定 0x1C if SDO_M is LOW
#define LSM9DS1_AG 0x6B // SPIアドレス設定 if SDO_AG is LOW

#define PRINT_CALCULATED
#define PRINT_SPEED 250   // 250 ms between prints
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

void webSocketEvent(WStype_t type, uint8_t *payload, size_t lenght)
{

    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[WSc] Disconnected!\n");
        break;

    case WStype_CONNECTED:
    {
        Serial.printf("[WSc] Connected to url: %s\n", payload);
    }
    break;

    case WStype_TEXT:
        Serial.printf("[WSc]Recv: %s\n", payload);

        // send data to back to Server
        //webSocket.sendTXT(payload, lenght);
        break;

    case WStype_BIN:
    {
        Serial.printf("[WSc] get binary lenght: %u\n", lenght);

        hexdump(payload, lenght);

        rec_data = (float *)payload;

        // echo data back to Server
        //webSocket.sendBIN(payload, lenght);
        break;
    }

    case WStype_ERROR:
        Serial.printf("We received an error \n");
        break;

    default:
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.print("\n\nStart\n");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
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

    imu.settings.device.commInterface = IMU_MODE_I2C;
    imu.settings.device.mAddress = LSM9DS1_M;
    imu.settings.device.agAddress = LSM9DS1_AG;

    if (!imu.begin())
    {
        // センサーエラー
        Serial.println("Failed to communicate with LSM9DS1.");
        Serial.println("Double-check wiring.");
        Serial.println("Default settings in this sketch will "
                       "work for an out of the box LSM9DS1 "
                       "Breakout, but may need to be modified "
                       "if the board jumpers are.");
        while (1)
            ;
    }
    Serial.printf("Sensor set");

    strip.begin();
    strip.show();
    Serial.printf("LED reset");
}

void printComData(float *data)
{
    Serial.print("heading, roll, pitch\n");
    for (int i = 0; i < NUM_OF_DATA; i++)
    {
        Serial.print(data[i]);
        Serial.print(" ");
    }
    Serial.println();
}

//-------------------------　メインループ　--------------------------------
void loop()
{
    static unsigned long last = 0;
    webSocket.loop();
    calcSensorData();
    send_data[0] = 3;
    send_data[1] = roll;
    send_data[2] = pitch;
    send_data[3] = heading;
    if (abs(millis() - last) > 10)
    {
        packet = (uint8_t *)send_data;
        webSocket.sendBIN(packet, packet_size);
        last = millis();
    }
    Serial.print("my data\n");
    printComData(send_data);
    setLED_roll(send_data, color1, true);
    Serial.print("received data\n");
    if (rec_data != NULL)
    {
        printComData(rec_data);
        setLED_roll(rec_data, color2, false);
    }
}
