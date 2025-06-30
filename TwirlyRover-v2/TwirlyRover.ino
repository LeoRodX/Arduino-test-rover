// Вертлявый ровер на Arduino Nano, избегающий препятствия. После сорока секунд активности десять секунд отдыхает.
// Новый скетч для  https://leorodx.github.io/Arduino-test-rover
// By Terre & Co, 2025

#include <stdlib.h> // Для random()

#define LED_PIN 13
#define TRIG_PIN 2
#define ECHO_PIN 3
#define LEFT_IN1 4
#define LEFT_IN2 5
#define RIGHT_IN3 6
#define RIGHT_IN4 7

// Настройки поведения
#define OBSTACLE_DISTANCE 40    // см до препятствия
#define AVOIDANCE_TIME 40000   // 60 сек активность
#define REST_TIME 10000        // 10 сек отдыха
#define MIN_SPEED 130          // Минимальная скорость
#define MAX_SPEED 160          // Максимальная скорость
#define TURN_VARIATION 300     // Вариация времени поворота (мс)
#define IDLE_MOVEMENT_TIME 3000 // Время между случайными движениями

enum BehaviorState {
  EXPLORING,          // Плавное движение вперед
  AVOIDING,           // Уклонение от препятствия
  RESTING,            // Отдых (стоянка)
  CURIOUS_TURN,       // Любопытный поворот
  BACKING_OFF         // Отъезд назад
};

BehaviorState currentState = EXPLORING;
unsigned long stateStartTime = 0;
unsigned long modeStartTime = 0;
unsigned long lastIdleMove = 0;
int currentSpeed = MIN_SPEED;
bool isActiveMode = true;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  
  randomSeed(analogRead(0)); // Инициализация генератора случайных чисел
  modeStartTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  float distance = getDistance();
  
  // Переключение между активным режимом и отдыхом
  if (isActiveMode && (currentTime - modeStartTime >= AVOIDANCE_TIME)) {
    switchToResting();
  } else if (!isActiveMode && (currentTime - modeStartTime >= REST_TIME)) {
    switchToActive();
  }

  // Основная логика поведения
  switch (currentState) {
    case EXPLORING:
      behaveExploring(currentTime, distance);
      break;
      
    case AVOIDING:
      behaveAvoiding(currentTime);
      break;
      
    case RESTING:
      behaveResting(currentTime);
      break;
      
    case CURIOUS_TURN:
      behaveCuriousTurn(currentTime);
      break;
      
    case BACKING_OFF:
      behaveBackingOff(currentTime, distance);
      break;
  }
  
  updateLED(currentTime);
}

void switchToResting() {
  isActiveMode = false;
  modeStartTime = millis();
  currentState = RESTING;
  stopMotors();
  Serial.println("Переход в режим отдыха");
}

void switchToActive() {
  isActiveMode = true;
  modeStartTime = millis();
  currentState = EXPLORING;
  currentSpeed = random(MIN_SPEED, MAX_SPEED);
  Serial.println("Переход в активный режим");
}

void behaveExploring(unsigned long currentTime, float distance) {
  // Плавное движение вперед с случайными изменениями
  if (currentTime - lastIdleMove > IDLE_MOVEMENT_TIME) {
    // Случайное изменение поведения
    int behaviorChoice = random(0, 100);
    
    if (behaviorChoice < 70) { // 70% продолжать движение
      currentSpeed = constrain(currentSpeed + random(-20, 20), MIN_SPEED, MAX_SPEED);
      moveForward(currentSpeed);
    } 
    else if (behaviorChoice < 90) { // 20% случайный поворот
      currentState = CURIOUS_TURN;
      stateStartTime = currentTime;
      if (random(0, 2)) {
        turnLeft(random(100, 100 + TURN_VARIATION));
      } else {
        turnRight(random(100, 100 + TURN_VARIATION));
      }
      return;
    } 
    else { // 10% остановка
      stopMotors();
      delay(random(100, 300));
    }
    
    lastIdleMove = currentTime;
  }
  
  // Проверка препятствий
  if (distance < OBSTACLE_DISTANCE && distance > 0) {
    currentState = AVOIDING;
    stateStartTime = currentTime;
    
    // Выбор стратегии избегания
    int avoidStrategy = random(0, 100);
    if (avoidStrategy < 40) { // 40% поворот налево
      turnLeft(random(300, 300 + TURN_VARIATION));
    } 
    else if (avoidStrategy < 80) { // 40% поворот направо
      turnRight(random(300, 300 + TURN_VARIATION));
    } 
    else { // 20% отъезд назад
      currentState = BACKING_OFF;
      moveBackward(MIN_SPEED);
    }
  } 
  else {
    moveForward(currentSpeed);
  }
}

void behaveAvoiding(unsigned long currentTime) {
  // После завершения времени поворота возвращаемся к исследованию
  if (currentTime - stateStartTime > TURN_VARIATION) {
    currentState = EXPLORING;
    currentSpeed = random(MIN_SPEED, MAX_SPEED);
  }
}

void behaveResting(unsigned long currentTime) {
  // Случайные подрагивания во время отдыха
  if (random(0, 1000) < 5) { // 0.5% chance
    int flicker = random(1, 4);
    for (int i = 0; i < flicker; i++) {
      moveForward(MIN_SPEED/2);
      delay(50);
      stopMotors();
      delay(100);
    }
  }
}

void behaveCuriousTurn(unsigned long currentTime) {
  if (currentTime - stateStartTime > TURN_VARIATION/2) {
    currentState = EXPLORING;
    currentSpeed = random(MIN_SPEED, MAX_SPEED);
  }
}

void behaveBackingOff(unsigned long currentTime, float distance) {
  if (distance > OBSTACLE_DISTANCE * 1.5 || currentTime - stateStartTime > 1000) {
    currentState = EXPLORING;
    currentSpeed = random(MIN_SPEED, MAX_SPEED);
  } else {
    moveBackward(constrain(currentSpeed - 20, MIN_SPEED/2, MAX_SPEED));
  }
}

void updateLED(unsigned long currentTime) {
  if (currentState == RESTING) {
    // Медленное дыхание в режиме отдыха
    int brightness = (currentTime / 20) % 255;
    analogWrite(LED_PIN, brightness < 128 ? brightness : 255 - brightness);
  } 
  else if (currentState == AVOIDING || currentState == BACKING_OFF) {
    // Частое мигание при избегании
    digitalWrite(LED_PIN, (currentTime % 200) < 100 ? HIGH : LOW);
  } 
  else {
    // Нормальное свечение при исследовании
    analogWrite(LED_PIN, currentState == EXPLORING ? 255 : 50);
  }
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

void moveForward(int speed) {
  analogWrite(LEFT_IN1, speed);
  analogWrite(LEFT_IN2, 0);
  analogWrite(RIGHT_IN3, speed);
  analogWrite(RIGHT_IN4, 0);
}

void moveBackward(int speed) {
  analogWrite(LEFT_IN1, 0);
  analogWrite(LEFT_IN2, speed);
  analogWrite(RIGHT_IN3, 0);
  analogWrite(RIGHT_IN4, speed);
}

void turnLeft(int duration) {
  analogWrite(LEFT_IN1, 0);
  analogWrite(LEFT_IN2, currentSpeed);
  analogWrite(RIGHT_IN3, currentSpeed);
  analogWrite(RIGHT_IN4, 0);
}

void turnRight(int duration) {
  analogWrite(LEFT_IN1, currentSpeed);
  analogWrite(LEFT_IN2, 0);
  analogWrite(RIGHT_IN3, 0);
  analogWrite(RIGHT_IN4, currentSpeed);
}

void stopMotors() {
  analogWrite(LEFT_IN1, 0);
  analogWrite(LEFT_IN2, 0);
  analogWrite(RIGHT_IN3, 0);
  analogWrite(RIGHT_IN4, 0);
}