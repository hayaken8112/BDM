
// n:index, m:number of LEDs (must be odd)
void setLED(uint8_t n, uint8_t m,uint32_t color){
  m = (m+1)/2;
  for (uint8_t i = 0; i < strip.numPixels(); i++) {
    if(n-m < i && i < n+m) {
      strip.setPixelColor(i, color);
    } else {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  strip.show();
}

void calcSensorData(){
  calcGyro();
  calcAccel();
  calcMag();
  calcAttitude(imu.ax, imu.ay, imu.az, -imu.my, -imu.mx, imu.mz);
}

void printSensorData(){
  printGyro();  // Print "G: gx, gy, gz"　　　シリアルモニタ表示用フォーマット
  printAccel(); // Print "A: ax, ay, az"
  printMag();   // Print "M: mx, my, mz"
  printAttitude();
  Serial.println();
  delay(PRINT_SPEED);
}

//******************************　Gyro DATA ****************************
void calcGyro()
{
  imu.readGyro();
  //------------　測位データ　Gyro/x,y,z　-----------
  gxVal = (imu.calcGyro(imu.gx));
  gyVal = (imu.calcGyro(imu.gy));
  gzVal = (imu.calcGyro(imu.gz));
}

void printGyro()
{
  Serial.print("G: ");
#ifdef PRINT_CALCULATED

  Serial.print(imu.calcGyro(imu.gx), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gy), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gz), 2);
  Serial.println(" deg/s");

#elif defined PRINT_RAW
  Serial.print(imu.gx);
  Serial.print(", ");
  Serial.print(imu.gy);
  Serial.print(", ");
  Serial.println(imu.gz);
#endif  
}
//-------------------　Accel DATA ----------------------
void calcAccel()
{
  // To read from the accelerometer, you must first call the
  // readAccel() function. When this exits, it'll update the
  // ax, ay, and az variables with the most current data.
  imu.readAccel();


//------------　測位データ　Gyro/x,y,z　-----------

  axVal = (imu.calcGyro(imu.ax));
  ayVal = (imu.calcGyro(imu.ay));
  azVal = (imu.calcGyro(imu.az));


}

void printAccel()
{
  Serial.print("A: ");
  #ifdef PRINT_CALCULATED

  Serial.print(imu.calcAccel(imu.ax), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.ay), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.az), 2);
  Serial.println(" g");
  #elif defined PRINT_RAW
  Serial.print(imu.ax);
  Serial.print(", ");
  Serial.print(imu.ay);
  Serial.print(", ");
  Serial.println(imu.az);
  #endif

}
//--------------　Mag DATA ------------------
void calcMag()
{
  imu.readMag();

//------------　測位データ　Gyro/x,y,z　-----------

  mxVal = (imu.calcGyro(imu.mx));
  myVal = (imu.calcGyro(imu.my));
  mzVal = (imu.calcGyro(imu.mz));

}

void printMag(){
  Serial.print("M: ");
  #ifdef PRINT_CALCULATED
  Serial.print(imu.calcMag(imu.mx), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.my), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.mz), 2);
  Serial.println(" gauss");
  #elif defined PRINT_RAW
  Serial.print(imu.mx);
  Serial.print(", ");
  Serial.print(imu.my);
  Serial.print(", ");
  Serial.println(imu.mz);
  #endif
}
//-----------------------------------------------------------------------------
void calcAttitude(float ax, float ay, float az, float mx, float my, float mz)
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

}

void printAttitude(){
  Serial.print("Pitch, Roll: ");
  Serial.print(pitch, 2);
  Serial.print(", ");
  Serial.println(roll, 2);
  Serial.print("Heading: ");
  Serial.println(heading, 2);
}

