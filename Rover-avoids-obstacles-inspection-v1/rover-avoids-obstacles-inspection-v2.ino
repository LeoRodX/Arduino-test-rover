// Ровер избегает препятствия и осматривается перед выбором нового направления

#include <stdlib.h> // Для random()

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
#define OBSTACLE_DISTANCE 40  // !!! Дистанция обнаружения препятствия (см)
#define BACKUP_TIME 500       // Время движения назад (мс)
#define TURN_TIME 250         // !!!Время поворота на 90 градусов (мс)
#define SCAN_STEP 90          // Шаг поворота при сканировании (градусы)
#define PANIC_TIME 3000       // Время "паники" (мс)
#define ERROR_THRESHOLD 3     // Порог ошибок для активации паники
#define MIN_CLEAR_DISTANCE 60 // !!! Минимальное приемлемое расстояние при сканировании

// Глобальные переменные
int errorCount = 0;                  // Счетчик последовательных ошибок
unsigned long lastObstacleTime = 0;  // Время последнего препятствия
bool isPanicking = false;            // Флаг режима паники

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
  randomSeed(analogRead(0)); // Инициализация генератора случайных чисел
}

void loop() {
  if (isPanicking) {
    panicMode();
    return;
  }

  float distance = getDistance();
  
  // Проверка на частые препятствия
  if (distance < OBSTACLE_DISTANCE) {
    handleObstacle();
  } 
  else {
    // Нормальное движение
    moveForward();
    digitalWrite(LED_PIN, LOW);
    errorCount = 0; // Сброс счетчика при успешном движении
  }
  
  delay(50);
}

void handleObstacle() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastObstacleTime < 2000) { // Препятствие появилось слишком быстро
    errorCount++;
  } else {
    errorCount = max(0, errorCount - 1); // Постепенный сброс счетчика
  }
  
  lastObstacleTime = currentTime;
  
  if (errorCount >= ERROR_THRESHOLD) {
    isPanicking = true;
    return;
  }
  
  // Стандартное избегание
  stopAllMotors();
  blinkLED(3, 100);
  smartAvoidance();
}

void smartAvoidance() {
  // 1. Движение назад
  moveBackward();
  delay(BACKUP_TIME);
  stopAllMotors();
  delay(300);
  
  // 2. Сканирование пространства
  int bestDirection = scanSurroundings();
  
  // 3. Поворот в лучшем направлении
  if (bestDirection == 1) {
    turnRight();
    delay(TURN_TIME);
  } else if (bestDirection == -1) {
    turnLeft();
    delay(TURN_TIME);
  } else {
    // Если оба направления плохие - разворот
    turnRight();
    delay(TURN_TIME * 2);
  }
  stopAllMotors();
  delay(300);
}

int scanSurroundings() {
  int leftDistance = 0;
  int rightDistance = 0;
  
  // Сканирование влево
  turnLeft();
  delay(TURN_TIME);
  stopAllMotors();
  delay(500); // Пауза для стабилизации
  leftDistance = getDistance();
  
  // Сканирование вправо (поворот назад + вправо)
  turnRight();
  delay(TURN_TIME * 2);
  stopAllMotors();
  delay(500);
  rightDistance = getDistance();
  
  // Возврат в исходное положение
  turnLeft();
  delay(TURN_TIME);
  stopAllMotors();
  delay(500);
  
  // Выбор направления
  if (leftDistance > rightDistance && leftDistance > MIN_CLEAR_DISTANCE) {
    return -1; // Влево
  } else if (rightDistance > MIN_CLEAR_DISTANCE) {
    return 1; // Вправо
  } else if (leftDistance > rightDistance) {
    return -1; // Выбираем лучшее из плохих
  } else {
    return 1;
  }
}

void panicMode() {
  unsigned long startTime = millis();
  blinkLED(10, 100); // Сигнал начала паники
  
  while (millis() - startTime < PANIC_TIME) {
    // Случайные движения
    switch (random(4)) {
      case 0: 
        moveForward(); 
        digitalWrite(LED_PIN, HIGH);
        break;
      case 1: 
        moveBackward(); 
        digitalWrite(LED_PIN, LOW);
        break;
      case 2: 
        turnLeft(); 
        digitalWrite(LED_PIN, millis() % 200 < 100);
        break;
      case 3: 
        turnRight(); 
        digitalWrite(LED_PIN, millis() % 200 < 100);
        break;
    }
    delay(random(200, 500));
  }
  
  // Завершение паники
  stopAllMotors();
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  
  // Сброс состояния
  isPanicking = false;
  errorCount = 0;
  lastObstacleTime = millis() + 5000; // Даем время на выход
}

// ===== БАЗОВЫЕ ФУНКЦИИ ===== //

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

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void blinkLED(int count, int delayTime) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayTime);
    digitalWrite(LED_PIN, LOW);
    delay(delayTime);
  }
}