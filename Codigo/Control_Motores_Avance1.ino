const int ENA_IZQ = 5, IN1_IZQ = 7, IN2_IZQ = 6;
const int ENB_DER = 11, IN3_DER = 4, IN4_DER = 3;

const int VEL = 150;  // si no arranca, sube a 180-200

void motores(int izq, int der) {
  digitalWrite(IN1_IZQ, izq > 0); digitalWrite(IN2_IZQ, izq < 0);
  digitalWrite(IN3_DER, der > 0); digitalWrite(IN4_DER, der < 0);
  analogWrite(ENA_IZQ, abs(izq));
  analogWrite(ENB_DER, abs(der));
}

void setup() {
  pinMode(ENA_IZQ, OUTPUT); pinMode(IN1_IZQ, OUTPUT); pinMode(IN2_IZQ, OUTPUT);
  pinMode(ENB_DER, OUTPUT); pinMode(IN3_DER, OUTPUT); pinMode(IN4_DER, OUTPUT);
  motores(0, 0);
  delay(3000);  // tiempo para soltar el robot antes de que arranque
}

void loop() {
  motores(VEL, VEL);   delay(2000);  // adelante
  motores(0, 0);       delay(500);
  motores(-VEL, -VEL); delay(2000);  // atrás
  motores(0, 0);       delay(500);
  motores(-VEL, VEL);  delay(1000);  // giro izquierda
  motores(0, 0);       delay(500);
  motores(VEL, -VEL);  delay(1000);  // giro derecha
  motores(0, 0);       delay(1500);
}
