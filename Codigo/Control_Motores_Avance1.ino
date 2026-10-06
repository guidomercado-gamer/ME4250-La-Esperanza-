#include <Wire.h>

// ═══════════════════════════════════════════════════════════
// 1. PINES DE MOTORES (Basado en tu hardware)
// ═══════════════════════════════════════════════════════════
const int ENA_IZQ = 5;    // PWM velocidad izquierda
const int IN1_IZQ = 7;    // Dirección 1 izquierda
const int IN2_IZQ = 6;    // Dirección 2 izquierda

const int ENB_DER = 11;   // PWM velocidad derecha
const int IN3_DER = 4;    // Dirección 1 derecha
const int IN4_DER = 3;    // Dirección 2 derecha

// ═══════════════════════════════════════════════════════════
// 2. PARÁMETROS DE CONTROL Y FILTRO
// ═══════════════════════════════════════════════════════════
const unsigned long LOOP_DT_MS = 10; // Lazo a 100Hz
const float COMP_ALPHA = 0.98;       // CORREGIDO: 98% giroscopio, 2% acelerómetro

// Ganancias iniciales (sintonizables en vivo)
float Kp = 35.0;
float Ki = 0.0;     // Empezamos en 0 para sintonizar
float Kd = 0.5;
float setpoint = 0.0; // Cambiará según tu centro de masa

const float INTEGRAL_LIMIT = 100.0;
const float INTEGRAL_ACTIVE_BAND = 5.0; 
const float INTEGRAL_RESET_ANGLE = 5.0; 
const float MAX_ANGLE = 25.0;
const float ANGLE_DEADBAND = 0.2; 

const int ZONA_MUERTA = 40;  // Fricción motores TT
const int PWM_MIN = 50;
const int PWM_MAX = 255;

// ═══════════════════════════════════════════════════════════
// 3. VARIABLES DE ESTADO Y HARDWARE
// ═══════════════════════════════════════════════════════════
const int MPU_ADDR = 0x68;
const float GYRO_SENS = 131.0;

int16_t ax, ay, az, gx, gy, gz;
float gx_offset = 0, gy_offset = 0, gz_offset = 0;
float pitch = 0.0, gyroPitchRate = 0.0, integral = 0.0, errorPrev = 0.0;
unsigned long lastTime, nextLoop = 0;
float dt;

// ═══════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  
  // Motores
  pinMode(ENA_IZQ, OUTPUT); pinMode(IN1_IZQ, OUTPUT); pinMode(IN2_IZQ, OUTPUT); 
  pinMode(ENB_DER, OUTPUT); pinMode(IN3_DER, OUTPUT); pinMode(IN4_DER, OUTPUT); 
  stopMotors();

  setupMPU();

  Serial.println(F("Calibrando MPU6050... NO MUEVAS EL ROBOT."));
  delay(1500);
  calibrateGyro();
  Serial.println(F("Calibracion completa."));
  
  readMPU6050();
  pitch = computeAccelPitch();
  errorPrev = setpoint - pitch;

  imprimirMenu();

  lastTime = millis();
  nextLoop = millis() + LOOP_DT_MS;
}

// ═══════════════════════════════════════════════════════════
// LOOP PRINCIPAL
// ═══════════════════════════════════════════════════════════
void loop() {
  verificarComandosSerial(); // Revisa si hay ajustes en vivo

  if (millis() < nextLoop) return;
  nextLoop += LOOP_DT_MS;

  readMPU6050();
  unsigned long now = millis();
  dt = (now - lastTime) / 1000.0;
  lastTime = now;

  updatePitch();

  // Corte de seguridad por caída inminente
  if (abs(pitch) > MAX_ANGLE) {
    stopMotors();
    integral = 0;
    errorPrev = setpoint - pitch;
    return;
  }

  // Anti-Windup en ángulos críticos
  if (abs(pitch) > INTEGRAL_RESET_ANGLE) integral = 0;

  float error = setpoint - pitch;

  // Zona muerta angular
  if (abs(error) < ANGLE_DEADBAND) {
    stopMotors();
    errorPrev = error;
    return;
  }

  float output = computePID(error);
  applyMotors(output);
}

// ═══════════════════════════════════════════════════════════
// FUNCIONES DE CONTROL (PID y MOTORES)
// ═══════════════════════════════════════════════════════════
float computePID(float error) {
  if (abs(error) < INTEGRAL_ACTIVE_BAND) {
    integral += error * dt;
    integral = constrain(integral, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);
  }
  float derivative = (error - errorPrev) / dt;
  errorPrev = error;
  return (Kp * error) + (Ki * integral) + (Kd * derivative);
}

int calcularPWM(float output) {
  int p = abs((int)output);
  if (p == 0) return 0;
  int mapeado = map(p, 0, PWM_MAX, ZONA_MUERTA, PWM_MAX); 
  return constrain(mapeado, ZONA_MUERTA, PWM_MAX);
}

void applyMotors(float output) {
  int pwm = calcularPWM(output);
  
  // output positivo = inclinado hacia adelante = robot debe avanzar
  if (output > 0) { 
    digitalWrite(IN1_IZQ, HIGH); digitalWrite(IN2_IZQ, LOW);
    digitalWrite(IN3_DER, HIGH); digitalWrite(IN4_DER, LOW);
  } else {
    digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, HIGH);
    digitalWrite(IN3_DER, LOW); digitalWrite(IN4_DER, HIGH);
  }
  analogWrite(ENA_IZQ, pwm);
  analogWrite(ENB_DER, pwm);
}

void stopMotors() {
  digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, LOW); analogWrite(ENA_IZQ, 0); 
  digitalWrite(IN3_DER, LOW); digitalWrite(IN4_DER, LOW); analogWrite(ENB_DER, 0); 
}

// ═══════════════════════════════════════════════════════════
// FUNCIONES MPU6050 Y SINTONIZACIÓN SERIAL
// ═══════════════════════════════════════════════════════════
void setupMPU() {
  Wire.begin();
  Wire.setClock(400000);
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x6B); Wire.write(0); Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x1A); Wire.write(0x03); Wire.endTransmission(true); // DLPF
}

void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x3B); Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);
  ax = Wire.read() << 8 | Wire.read(); ay = Wire.read() << 8 | Wire.read(); az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read();
  gx = Wire.read() << 8 | Wire.read(); gy = Wire.read() << 8 | Wire.read(); gz = Wire.read() << 8 | Wire.read();
}

void calibrateGyro() {
  long sumX = 0, sumY = 0, sumZ = 0;
  for (int i = 0; i < 3000; i++) { readMPU6050(); sumX += gx; sumY += gy; sumZ += gz; delay(2); }
  gx_offset = (float)sumX / 3000.0; gy_offset = (float)sumY / 3000.0; gz_offset = (float)sumZ / 3000.0;
}

float computeAccelPitch() {
  float denom = sqrt((float)ay * (float)ay + (float)az * (float)az);
  if (denom < 0.0001) denom = 0.0001;
  return atan2(-(float)ax, denom) * 180.0 / PI;
}

void updatePitch() {
  gyroPitchRate = ((float)gy - gy_offset) / GYRO_SENS;
  float accelPitch = computeAccelPitch();
  pitch = COMP_ALPHA * (pitch + gyroPitchRate * dt) + (1.0 - COMP_ALPHA) * accelPitch;
}

void imprimirMenu() {
  Serial.println(F("\n--- SINTONIZACION PID EN VIVO ---"));
  Serial.println(F("Usa estas letras en el monitor (mayuscula sube, minuscula baja):"));
  Serial.println(F("P/p -> Ajustar Kp  |  I/i -> Ajustar Ki"));
  Serial.println(F("D/d -> Ajustar Kd  |  S/s -> Ajustar Setpoint"));
  Serial.println(F("V -> Ver valores actuales y Pitch\n"));
}

void verificarComandosSerial() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    switch (c) {
      case 'P': Kp += 1.0; break; case 'p': Kp -= 1.0; break;
      case 'I': Ki += 0.5; break; case 'i': Ki -= 0.5; break;
      case 'D': Kd += 0.1; break; case 'd': Kd -= 0.1; break;
      case 'S': setpoint += 0.5; break; case 's': setpoint -= 0.5; break;
      case 'V': case 'v':
        Serial.print(F("Pitch actual: ")); Serial.print(pitch);
        Serial.print(F(" | Setpoint: ")); Serial.print(setpoint);
        Serial.print(F(" | Kp: ")); Serial.print(Kp);
        Serial.print(F(" | Ki: ")); Serial.print(Ki);
        Serial.print(F(" | Kd: ")); Serial.println(Kd);
        break;
    }
  }
}
