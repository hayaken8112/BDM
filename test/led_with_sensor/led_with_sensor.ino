#include <Arduino.h>
#include <ESP8266WiFi.h>                      //https://github.com/esp8266/Arduino
#include <Wire.h>                             //I2Cライブラリ
#include <SparkFunLSM9DS1.h>                  //https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
#include <Adafruit_NeoPixel.h>
#include <Math.h>
#define MAX_VAL 20  // 0 to 255 for brightness
#define DELAY_TIME 50 
#define DELAY_TIME2 20
#define ADAddr 0x48
Adafruit_NeoPixel mystrip = Adafruit_NeoPixel(15, 0, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel yourstrip = Adafruit_NeoPixel(15, 14, NEO_GRB + NEO_KHZ800);

//-------------------------------------------------------------------------
float gxVal = 0;                                //ジャイロｘ軸用データーレジスタ
float gyVal = 0;                                //ジャイロｙ軸用データーレジスタ
float gzVal = 0;                                //ジャイロｚ軸用データーレジスタ
float axVal = 0;                                //Axis ｘ用データーレジスタ
float ayVal = 0;                                //Axis ｙ用データーレジスタ
float azVal = 0;                                //Axis ｚ用データーレジスタ
float mxVal = 0;                                //Mag x 用データーレジスタ
float myVal = 0;                                //Mag ｙ 用データーレジスタ
float mzVal = 0;                                //Mag x 用データーレジスタ
float hedVal = 0;                               //Hedding 用データーレジスタ

//------------------------------------------------------------------------
float heading = 0;
float roll = 0;
float pitch = 0;

uint8_t current_index = 0;
uint8_t previous_index = 0;
LSM9DS1 imu;

#define LSM9DS1_M  0x1E // SPIアドレス設定 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // SPIアドレス設定 if SDO_AG is LOW

#define PRINT_CALCULATED
#define PRINT_SPEED 250 // 250 ms between prints
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.


WiFiServer server(80);
WiFiClient client;

//----------------------　setup　---------------------------
void setup() {
  Serial.begin(115200);

  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;

  if (!imu.begin())                                     //センサ接続エラー時の表示

  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
    while (1)
      ;
  }

  mystrip.begin();
  mystrip.show(); // Initialize all pixels to 'off'
  yourstrip.begin();
  yourstrip.show();

}

//-------------------------　メインループ　--------------------------------
void loop(){
  Serial.println("loop");
  printSensorData();
  headLED();
}

void headLED(){
  current_index = (uint8_t)((heading + 180)/24);
  uint8_t distance = 0;
  Serial.println(current_index);
  uint8_t brightness = 0;
  for(uint8_t i = 0; i<15; i++){
    distance = abs(i - current_index);
    if(distance > 7){
      distance = 15 - distance; 
    }
    brightness = (7 -distance)*(7-distance);
    mystrip.setPixelColor(i, mystrip.Color(brightness,0,0));
  }
  mystrip.show();
  //previous_index = current_index;
}

void rollLED(){
  current_index = (uint8_t)((roll + 90)/12);
  Serial.println(current_index);
  if (previous_index < current_index) {
    for (uint8_t i = previous_index; i <= current_index; i++){
      setLED(i, 3, mystrip.Color(MAX_VAL, 0, 0));
      delay(30);
    }
  } else {
    for (uint8_t i = previous_index; i >= current_index; i--){
      setLED(i, 3, mystrip.Color(MAX_VAL, 0, 0));
      delay(30);
    }
  }
  //setLED(num, 3,strip.Color(MAX_VAL, 0, 0));
  previous_index = current_index;
}

// n:index, m:number of LEDs (must be odd)
void setLED(uint8_t n, uint8_t m,uint32_t color){
  m = (m+1)/2;
  for (uint8_t i = 0; i < mystrip.numPixels(); i++) {
    if(n-m < i && i < n+m) {
      mystrip.setPixelColor(i, color);
    } else {
      mystrip.setPixelColor(i, mystrip.Color(0,0,0));
    }
  }
  mystrip.show();
}

void printSensorData(){
  printGyro();  // Print "G: gx, gy, gz"　　　シリアルモニタ表示用フォーマット
  printAccel(); // Print "A: ax, ay, az"
  printMag();   // Print "M: mx, my, mz"
  printAttitude(imu.ax, imu.ay, imu.az, -imu.my, -imu.mx, imu.mz);
  Serial.println();
  delay(PRINT_SPEED);
}

//******************************　Gyro DATA ****************************
void printGyro()
{

  imu.readGyro();

  Serial.print("G: ");
#ifdef PRINT_CALCULATED

  Serial.print(imu.calcGyro(imu.gx), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gy), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gz), 2);
  Serial.println(" deg/s");

  //------------　測位データ　Gyro/x,y,z　-----------

  gxVal = (imu.calcGyro(imu.gx));
  gyVal = (imu.calcGyro(imu.gy));
  gzVal = (imu.calcGyro(imu.gz));

#elif defined PRINT_RAW
  Serial.print(imu.gx);
  Serial.print(", ");
  Serial.print(imu.gy);
  Serial.print(", ");
  Serial.println(imu.gz);
#endif

}
//-------------------　Accel DATA ----------------------
void printAccel()
{
  // To read from the accelerometer, you must first call the
  // readAccel() function. When this exits, it'll update the
  // ax, ay, and az variables with the most current data.
  imu.readAccel();

  Serial.print("A: ");
#ifdef PRINT_CALCULATED

  Serial.print(imu.calcAccel(imu.ax), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.ay), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.az), 2);
  Serial.println(" g");
//------------　測位データ　Gyro/x,y,z　-----------

  axVal = (imu.calcGyro(imu.ax));
  ayVal = (imu.calcGyro(imu.ay));
  azVal = (imu.calcGyro(imu.az));

#elif defined PRINT_RAW
  Serial.print(imu.ax);
  Serial.print(", ");
  Serial.print(imu.ay);
  Serial.print(", ");
  Serial.println(imu.az);
#endif

}
//--------------　Mag DATA ------------------
void printMag()
{

  imu.readMag();
  Serial.print("M: ");
#ifdef PRINT_CALCULATED

  Serial.print(imu.calcMag(imu.mx), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.my), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.mz), 2);
  Serial.println(" gauss");
//------------　測位データ　Gyro/x,y,z　-----------

  mxVal = (imu.calcGyro(imu.mx));
  myVal = (imu.calcGyro(imu.my));
  mzVal = (imu.calcGyro(imu.mz));

#elif defined PRINT_RAW
  Serial.print(imu.mx);
  Serial.print(", ");
  Serial.print(imu.my);
  Serial.print(", ");
  Serial.println(imu.mz);

#endif

}
//-----------------------------------------------------------------------------
void printAttitude(float ax, float ay, float az, float mx, float my, float mz)
{
  roll = atan2(ay, az);
  pitch = atan2(-ax, sqrt(ay * ay + az * az));
  //float heading;
  if (my == 0)
    heading = (mx < 0) ? 180.0 : 0;
  else
    heading = atan2(mx, my);

  heading -= DECLINATION * PI / 180;

  if (heading > PI) heading -= (2 * PI);
  else if (heading < -PI) heading += (2 * PI);
  else if (heading < 0) heading += 2 * PI;

  // Convert everything from radians to degrees:
  heading *= 180.0 / PI;
  pitch *= 180.0 / PI;
  roll  *= 180.0 / PI;

  Serial.print("Pitch, Roll: ");
  Serial.print(pitch, 2);
  Serial.print(", ");
  Serial.println(roll, 2);
  Serial.print("Heading: ");
  Serial.println(heading, 2);

}
