// ═══════════════════════════════════════════════════════════
// PINES DE MOTORES (Basado en tu hardware)
// ═══════════════════════════════════════════════════════════
const int ENA_IZQ = 5;    // PWM velocidad izquierda
const int IN1_IZQ = 7;    // Dirección 1 izquierda
const int IN2_IZQ = 6;    // Dirección 2 izquierda

const int ENB_DER = 11;   // PWM velocidad derecha
const int IN3_DER = 4;    // Dirección 1 derecha
const int IN4_DER = 3;    // Dirección 2 derecha

// ═══════════════════════════════════════════════════════════
// PARÁMETROS DE LOS MOTORES TT (1:48)
// ═══════════════════════════════════════════════════════════
const int ZONA_MUERTA = 40;  // PWM mínimo donde el motor hace ruido pero no gira
const int PWM_MIN = 50;      // Potencia mínima para empezar a rodar
const int PWM_MAX = 255;     // Máxima potencia
int velocidadPrueba = 150;   // Velocidad estándar para la prueba motriz

// ═══════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // Configuración de pines de salida para los motores
  pinMode(ENA_IZQ, OUTPUT); pinMode(IN1_IZQ, OUTPUT); pinMode(IN2_IZQ, OUTPUT); 
  pinMode(ENB_DER, OUTPUT); pinMode(IN3_DER, OUTPUT); pinMode(IN4_DER, OUTPUT); 

  // Iniciar apagado por seguridad
  Detener(); 

  Serial.println(F("\n================================="));
  Serial.println(F("  AVANCE 1: TEST MOTRIZ DIFERENCIAL"));
  Serial.println(F("================================="));
  Serial.println(F("Asegurate de tener alimentacion en el Puente H (bateria externa)."));
  Serial.println(F("Comandos (Escribe y presiona Enter):"));
  Serial.println(F("  W -> Adelante"));
  Serial.println(F("  S -> Atras"));
  Serial.println(F("  A -> Giro sobre el eje (Izquierda)"));
  Serial.println(F("  D -> Giro sobre el eje (Derecha)"));
  Serial.println(F("  X -> Detener"));
  Serial.println(F("  + -> Aumentar velocidad prueba (+10)"));
  Serial.println(F("  - -> Disminuir velocidad prueba (-10)"));
  Serial.println(F("=================================\n"));
}

// ═══════════════════════════════════════════════════════════
// LOOP PRINCIPAL (Escuchando Comandos)
// ═══════════════════════════════════════════════════════════
void loop() {
  if (Serial.available() > 0) {
    char comando = Serial.read();
    comando = toupper(comando); // Convertir a mayúscula para ser robustos a errores de tipeo

    switch (comando) {
      case 'W':
        Adelante(velocidadPrueba);
        Serial.print(F("Moviendo: ADELANTE | Velocidad: ")); Serial.println(velocidadPrueba);
        break;
      case 'S':
        Atras(velocidadPrueba);
        Serial.print(F("Moviendo: ATRAS    | Velocidad: ")); Serial.println(velocidadPrueba);
        break;
      case 'A':
        GiroIzquierda(velocidadPrueba);
        Serial.print(F("Moviendo: IZQ (Giro sobre eje) | Vel: ")); Serial.println(velocidadPrueba);
        break;
      case 'D':
        GiroDerecha(velocidadPrueba);
        Serial.print(F("Moviendo: DER (Giro sobre eje) | Vel: ")); Serial.println(velocidadPrueba);
        break;
      case 'X':
        Detener();
        Serial.println(F("Moviendo: DETENIDO"));
        break;
      case '+':
        velocidadPrueba += 10;
        if (velocidadPrueba > PWM_MAX) velocidadPrueba = PWM_MAX;
        Serial.print(F("Velocidad de prueba ajustada a: ")); Serial.println(velocidadPrueba);
        break;
      case '-':
        velocidadPrueba -= 10;
        if (velocidadPrueba < PWM_MIN) velocidadPrueba = PWM_MIN;
        Serial.print(F("Velocidad de prueba ajustada a: ")); Serial.println(velocidadPrueba);
        break;
    }
  }
}

// ═══════════════════════════════════════════════════════════
// FUNCIONES BASE DE CONTROL DE MOTORES
// ═══════════════════════════════════════════════════════════
int calcularPWM(int potencia) {
  int p = abs(potencia);
  if (p < ZONA_MUERTA) return 0; // Si el comando es tan bajo que no vence la fricción, se apaga.
  
  // Si pedimos una potencia de 100, la mapea para que asuma que el rango real empieza en la zona muerta
  int mapeado = map(p, ZONA_MUERTA, PWM_MAX, PWM_MIN, PWM_MAX); 
  return constrain(mapeado, 0, PWM_MAX);
}

void setMotorIzquierdo(int potencia) {
  int pwm = calcularPWM(potencia);
  if (potencia > 0) {
    digitalWrite(IN1_IZQ, HIGH); digitalWrite(IN2_IZQ, LOW);
  } else if (potencia < 0) {
    digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, HIGH);
  } else {
    digitalWrite(IN1_IZQ, LOW); digitalWrite(IN2_IZQ, LOW);
  }
  analogWrite(ENA_IZQ, pwm);
}

void setMotorDerecho(int potencia) {
  int pwm = calcularPWM(potencia);
  if (potencia > 0) {
    digitalWrite(IN3_DER, HIGH); digitalWrite(IN4_DER, LOW);
  } else if (potencia < 0) {
    digitalWrite(IN3_DER, LOW); digitalWrite(IN4_DER, HIGH);
  } else {
    digitalWrite(IN3_DER, LOW); digitalWrite(IN4_DER, LOW);
  }
  analogWrite(ENB_DER, pwm);
}

// ═══════════════════════════════════════════════════════════
// FUNCIONES CINEMÁTICAS (Dirección)
// ═══════════════════════════════════════════════════════════
void Adelante(int vel) { 
  setMotorIzquierdo(vel); 
  setMotorDerecho(vel); 
}

void Atras(int vel) { 
  setMotorIzquierdo(-vel); 
  setMotorDerecho(-vel); 
}

// Giros sobre su propio eje (una rueda adelante, otra atrás)
void GiroIzquierda(int vel) { 
  setMotorIzquierdo(-vel); 
  setMotorDerecho(vel); 
}

void GiroDerecha(int vel) { 
  setMotorIzquierdo(vel); 
  setMotorDerecho(-vel); 
}

void Detener() { 
  setMotorIzquierdo(0); 
  setMotorDerecho(0); 
}
