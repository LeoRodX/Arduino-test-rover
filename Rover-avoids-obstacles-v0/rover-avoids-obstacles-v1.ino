// Скетч ровера объезжающего препятствия 

// Пин для светодиода
#define LED_PIN 13

// Пины для ультразвукового датчика HC-SR04
#define TRIG_PIN 2
#define ECHO_PIN 3

// Пины для драйвера двигателей MX1508
#define LEFT_IN1 4   // Левый двигатель +
#define LEFT_IN2 5   // Левый двигатель -
#define RIGHT_IN3 6  // Правый двигатель +
#define RIGHT_IN4 7  // Правый двигатель -

// Константы
#define STOP_DISTANCE 30      // Дистанция остановки (см)
#define OBSTACLE_DISTANCE 20  // Дистанция обнаружения препятствия (см)
#define BACKUP_TIME 500       // Время движения назад (мс)
#define TURN_TIME 500         // Время поворота (мс)

void setup() {
  // Инициализация пинов
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  
  // Короткий сигнал готовности
  blinkLED(2, 200);
}

void loop() {
  float distance = getDistance();
  
  if (distance > STOP_DISTANCE) {
    // Свободный путь - движение вперед
    moveForward();
    digitalWrite(LED_PIN, LOW);
  } 
  else {
    // Препятствие обнаружено
    stopAllMotors();
    blinkLED(3, 100);
    avoidObstacle();
  }
  
  delay(50);
}

void avoidObstacle() {
  // 1. Движение назад
  moveBackward();
  delay(BACKUP_TIME);
  stopAllMotors();
  delay(300);
  
  // 2. Поворот (выбираем случайное направление)
  if (random(2) == 0) {
    turnRight();
  } else {
    turnLeft();
  }
  delay(TURN_TIME);
  stopAllMotors();
  delay(300);
}

// Базовые функции движения
void moveForward() {
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, HIGH);
  digitalWrite(RIGHT_IN4, LOW);
}

void moveBackward() {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, HIGH);
}

void turnRight() {
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, HIGH);
}

void turnLeft() {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN3, HIGH);
  digitalWrite(RIGHT_IN4, LOW);
}

void stopAllMotors() {
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, LOW);
}

// Работа с дальномером
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

// Индикация
void blinkLED(int count, int delayTime) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayTime);
    digitalWrite(LED_PIN, LOW);
    delay(delayTime);
  }
}