#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>            //I2Cライブラリ
#include <SparkFunLSM9DS1.h> //https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
#include <Adafruit_NeoPixel.h>

// ----------------- WiFi settings -------------------
//IP
IPAddress ip(192, 168, 11, 31);
IPAddress gateway(192, 168, 11, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 11, 1);
int port = 81;

//ssid, pass
const char *ssid = "Buffalo-G-95F5";
const char *password = "bnhcsau8trtk3";

WebSocketsServer webSocket = WebSocketsServer(port);

// ------------------- LED settings -----------------
#define MAX_VAL 20 // 0 to 255 for brightness
#define DELAY_TIME 50
#define DELAY_TIME2 20
#define ADAddr 0x48

Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, 0, NEO_GRB + NEO_KHZ800); // 0番ピン　自分

uint32_t color1 = strip.Color(MAX_VAL, 0, 0); // red
uint32_t color2 = strip.Color(0, MAX_VAL, 0); // green
uint32_t color3 = strip.Color(0,0,MAX_VAL); // brue

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

//------------------------------------------------------------------------
LSM9DS1 imu;

// ---------------------- data settings ----------------
#define NUM_OF_DATA 4

uint8_t *packet;
float send_data[NUM_OF_DATA];
float heading = 0;
float roll = 0;
float pitch = 0;
int mycolor = 0;

float rec_data[NUM_OF_DATA];

size_t packet_size = NUM_OF_DATA * sizeof(float);

#define LSM9DS1_M 0x1E  // SPIアドレス設定 0x1C if SDO_M is LOW
#define LSM9DS1_AG 0x6B // SPIアドレス設定 if SDO_AG is LOW

#define PRINT_CALCULATED
#define PRINT_SPEED 250   // 250 ms between prints
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{

    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;

    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    }
    break;

    case WStype_TEXT:
        Serial.printf("[%u] get Text: %s\n", num, payload);

        // echo data back to browser
        webSocket.sendTXT(num, "ok", length);
        // send data to all connected clients 全てのクライアントへ送信
        //webSocket.broadcastTXT(payload, length);
        break;

    case WStype_BIN:
    {

        Serial.printf("[%u] get binary length: %u\n", num, length);

        hexdump(payload, length);
        float * temp = (float *)payload;
        for (int i = 0; i < NUM_OF_DATA; i++) {
          rec_data[i] = temp[i];
        }
        //rec_data = (float *)payload;

        // echo data back to browser
        //webSocket.sendBIN(num, payload, length);
        break;
    }

    case WStype_ERROR:
        break;

    default:
        break;
    }
}

//----------------------　setup　---------------------------
void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.print("\n\nStart\n");

    WiFi.config(ip, gateway, subnet, DNS); //static ip
    delay(100);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(500);
    }
    Serial.println();
    Serial.printf("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.printf("Server Started\n");

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
    send_data[0] = 4;
    send_data[1] = roll;
    send_data[2] = pitch;
    //send_data[3] = heading;
    send_data[3] = (float)mycolor;
    if (abs(millis() - last) > 10)
    {
        packet = (uint8_t *)send_data;
        webSocket.sendBIN(0, packet, packet_size);
        last = millis();
    }
    Serial.print("my data\n");
    printComData(send_data);
    setLED_roll(send_data, color1, true);
    Serial.println();
    Serial.print("received data\n");

    if (rec_data != NULL)
    {
        printComData(rec_data);
        setLED_roll(rec_data, color2, false);
    }
    changeColor();
}
