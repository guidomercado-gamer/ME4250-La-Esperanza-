// Motor Izquierdo
const int ENA_IZQ = 5;    // PWM velocidad izquierda (Pin)
const int IN1_IZQ = 7;    // Dirección 1 izquierda
const int IN2_IZQ = 6;    // Dirección 2 izquierda

// Motor Derecho
const int ENB_DER = 11;   // PWM velocidad derecha (Pin)
const int IN3_DER = 4;    // Dirección 1 derecha
const int IN4_DER = 3;    // Dirección 2 derecha

// Parámetros de fricción mecánica y saturación (motores TT 1:48)
const int ZONA_MUERTA = 40;  // Umbral mínimo del motor
const int PWM_MIN = 50;      // Potencia mínima efectiva de arranque
const int PWM_MAX = 255;     // Saturación máxima de ciclo de trabajo
int velocidadPrueba = 170;

int calcularPWM(int potencia) {
  int p = abs(potencia);
  if (p < ZONA_MUERTA) return 0; // Si no supera la fricción, se apaga
  int mapeado = map(p, ZONA_MUERTA, PWM_MAX, PWM_MIN, PWM_MAX); 
  return constrain(mapeado, 0, PWM_MAX);
}

void Adelante(int potencia) {
  int pwm = calcularPWM(potencia);
  digitalWrite(IN1_IZQ, HIGH); digitalWrite(IN2_IZQ, LOW);  analogWrite(ENA_IZQ, pwm); 
  digitalWrite(IN3_DER, HIGH); digitalWrite(IN4_DER, LOW);  analogWrite(ENB_DER, pwm); 
}

void Atras(int potencia) {
  int pwm = calcularPWM(potencia);
  digitalWrite(IN1_IZQ, LOW);  digitalWrite(IN2_IZQ, HIGH); analogWrite(ENA_IZQ, pwm); 
  digitalWrite(IN3_DER, LOW);  digitalWrite(IN4_DER, HIGH); analogWrite(ENB_DER, pwm); 
}

void GiroIzquierda(int potencia) {
  int pwm = calcularPWM(potencia);
  digitalWrite(IN1_IZQ, LOW);  digitalWrite(IN2_IZQ, HIGH); analogWrite(ENA_IZQ, pwm); 
  digitalWrite(IN3_DER, HIGH); digitalWrite(IN4_DER, LOW);  analogWrite(ENB_DER, pwm); 
}

void GiroDerecha(int potencia) {
  int pwm = calcularPWM(potencia);
  digitalWrite(IN1_IZQ, HIGH); digitalWrite(IN2_IZQ, LOW);  analogWrite(ENA_IZQ, pwm); 
  digitalWrite(IN3_DER, LOW);  digitalWrite(IN4_DER, HIGH); analogWrite(ENB_DER, pwm); 
}

void Detener() {
  digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, LOW); analogWrite(ENA_IZQ, 0); 
  digitalWrite(IN3_DER, LOW); digitalWrite(IN4_DER, LOW); analogWrite(ENB_DER, 0); 
}

// ─── 3. SETUP: INICIALIZACIÓN ───
void setup() {
  Serial.begin(115200); //

  pinMode(ENA_IZQ, OUTPUT); pinMode(IN1_IZQ, OUTPUT); pinMode(IN2_IZQ, OUTPUT); 
  pinMode(ENB_DER, OUTPUT); pinMode(IN3_DER, OUTPUT); pinMode(IN4_DER, OUTPUT); 

  Detener(); // Inicia apagado por seguridad[cite: 1]

  Serial.println(F("\n--- TEST MOTRIZ AVANCE ) ---")); //[cite: 1, 4]
  Serial.println(F("Comandos: W (Adelante), S (Atras), A (GiroIzq), D (GiroDer), X (Detener)")); 
}
