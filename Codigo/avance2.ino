// ===== AVANCE 2: ROBOT DIFERENCIAL (curvas + aceleracion suave) =====

// Conexion Arduino -> Puente H
// 5  -> ENA (velocidad izquierda, PWM)
// 7  -> IN1
// 6  -> IN2
// 11 -> ENB (velocidad derecha, PWM)
// 4  -> IN3
// 3  -> IN4

const int ENA_IZQ = 5;
const int IN1_IZQ = 7;
const int IN2_IZQ = 6;
const int ENB_DER = 11;
const int IN3_DER = 4;
const int IN4_DER = 3;

const int VEL = 150;  // si no arranca, sube a 180-200

// Positivo = adelante, negativo = atras, 0 = detenido
void motores(int izq, int der) {
  digitalWrite(IN1_IZQ, izq > 0); digitalWrite(IN2_IZQ, izq < 0);
  digitalWrite(IN3_DER, der > 0); digitalWrite(IN4_DER, der < 0);
  analogWrite(ENA_IZQ, abs(izq));
  analogWrite(ENB_DER, abs(der));
}

// Acelera suavemente desde 0 hasta la velocidad pedida
void acelerar(int izq, int der) {
  for (int i = 0; i <= 10; i++) {
    motores(izq * i / 10, der * i / 10);
    delay(50);
  }
}

// Frena suavemente hasta detenerse
void frenar(int izq, int der) {
  for (int i = 10; i >= 0; i--) {
    motores(izq * i / 10, der * i / 10);
    delay(50);
  }
  delay(300);
}

void setup() {
  pinMode(ENA_IZQ, OUTPUT); pinMode(IN1_IZQ, OUTPUT); pinMode(IN2_IZQ, OUTPUT);
  pinMode(ENB_DER, OUTPUT); pinMode(IN3_DER, OUTPUT); pinMode(IN4_DER, OUTPUT);
  motores(0, 0);
  delay(3000);  // tiempo para soltar el robot antes de que arranque
}

void loop() {
  // 1. Recto
  acelerar(VEL, VEL);
  delay(1500);
  frenar(VEL, VEL);

  // 2. Curva a la derecha
  acelerar(VEL, VEL / 3);
  delay(2000);
  frenar(VEL, VEL / 3);

  // 3. Curva a la izquierda
  acelerar(VEL / 3, VEL);
  delay(2000);
  frenar(VEL / 3, VEL);

  // 4. Giro sobre su eje
  acelerar(-VEL, VEL);
  delay(1000);
  frenar(-VEL, VEL);

  // 5. Zigzag
  for (int i = 0; i < 3; i++) {
    motores(VEL, VEL / 3);  delay(600);
    motores(VEL / 3, VEL);  delay(600);
  }
  motores(0, 0);
  delay(2000);
}
