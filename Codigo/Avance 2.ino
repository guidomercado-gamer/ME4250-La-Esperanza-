#include <Wire.h>
const int MPU_ADDR = 0x68;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t gx_offset = 0, gy_offset = 0, gz_offset = 0;
float roll = 0, pitch = 0, yaw = 0;
float pitchAcel = 0;        
unsigned long lastTime;
float dt;
float puntoEquilibrio = 0;         
float zonaMuerta = 3; 

const int ENA_IZQ = 5;
const int IN1_IZQ = 7;
const int IN2_IZQ = 6;
const int ENB_DER = 11;
const int IN3_DER = 4;
const int IN4_DER = 3;

const int VEL = 150;               

void motores(int izq, int der) {
  digitalWrite(IN1_IZQ, izq > 0);
  digitalWrite(IN2_IZQ, izq < 0);
  digitalWrite(IN3_DER, der > 0);
  digitalWrite(IN4_DER, der < 0);
  analogWrite(ENA_IZQ, abs(izq));
  analogWrite(ENB_DER, abs(der));
}


void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // Skip temperature
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

void calibrateGyro() {
  long sumX = 0, sumY = 0, sumZ = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    readMPU6050();
    sumX += gx;
    sumY += gy;
    sumZ += gz;
    delay(5);
  }

  gx_offset = sumX / samples;
  gy_offset = sumY / samples;
  gz_offset = sumZ / samples;
}


void setup() {
  pinMode(ENA_IZQ, OUTPUT);
  pinMode(IN1_IZQ, OUTPUT);
  pinMode(IN2_IZQ, OUTPUT);
  pinMode(ENB_DER, OUTPUT);
  pinMode(IN3_DER, OUTPUT);
  pinMode(IN4_DER, OUTPUT);
  motores(0, 0);

  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); 
  Wire.write(0);
  Wire.endTransmission(true);
  delay(3000);                     
  calibrateGyro();
  
  readMPU6050();
  float denominadorInicial = sqrt((float)ay * ay + (float)az * az);
  pitch = atan2(-(float)ax, denominadorInicial) * 180.0 / PI;

  lastTime = millis();
}

void loop() {
  readMPU6050();

  unsigned long now = millis();
  dt = (now - lastTime) / 1000.0;
  lastTime = now;

  
  float gyroZ = (gz - gz_offset) / 131.0;
  yaw += gyroZ * dt;

  // Roll y pitch con el acelerometro
  float ax_f = (float)ax;
  float ay_f = (float)ay;
  float az_f = (float)az;

  float denominator = sqrt(ay_f * ay_f + az_f * az_f);
  if (denominator < 0.0001) denominator = 0.0001;

  roll = atan2(ay_f, az_f) * 180.0 / PI;
  pitchAcel = atan2(-ax_f, denominator) * 180.0 / PI;

  
  float gyroY = (gy - gy_offset) / 131.0;

  
  pitch = 0.98 * (pitch + gyroY * dt) + 0.02 * pitchAcel;

  
  float error = pitch - puntoEquilibrio;

  if (error > zonaMuerta) {
    motores(VEL, VEL);             
  }
  else if (error < -zonaMuerta) {
    motores(-VEL, -VEL);           
  }
  else {
    motores(0, 0);                 
  }
  delay(5000);
}
