// Ровер на Arduino Nano и mx1508  воспроизводит без бизера мелодию Бумер и делает разворот.
// By Terre & Co, 2025

#define LED_PIN 13 // встроенный в ардуино
#define LEFT_IN1 4 // Ищем на mx1508 in1-in4 или аналогичную маркировку, 4,5,6,7 - D-пины на ардуино
#define LEFT_IN2 5
#define RIGHT_IN3 6
#define RIGHT_IN4 7

// Определения нот
#define NOTE_REST 0
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988

// Мелодия "Бумер (Мобильник)"
int melody[] = {
  NOTE_E5, NOTE_G5, NOTE_REST, NOTE_G5, NOTE_E5, NOTE_REST,
  NOTE_A5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_A5, NOTE_B5
};

int durations[] = {
  8, 4, 4, 8, 4, 4,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 4
};

int noteCount = 16;
int tempo = 140;
const int turnDuration = 865; // Длительность разворота (подберите экспериментально)

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  
  // Инициализация всех выходов в LOW
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, LOW);
  digitalWrite(LED_PIN, LOW);
}

void playTone(int frequency, long duration) {
  if (frequency == NOTE_REST) {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);
    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(duration);
    return;
  }

  long period = 1000000L / frequency;
  long pulseWidth = period / 2;
  long elapsedTime = 0;
  digitalWrite(LED_PIN, HIGH);

  while (elapsedTime < duration * 1000L) {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);
    digitalWrite(RIGHT_IN3, HIGH);
    digitalWrite(RIGHT_IN4, LOW);
    delayMicroseconds(pulseWidth);
    
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);
    digitalWrite(RIGHT_IN3, LOW);
    digitalWrite(RIGHT_IN4, HIGH);
    delayMicroseconds(pulseWidth);
    
    elapsedTime += period;
  }
  
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, LOW);
  digitalWrite(LED_PIN, LOW);
}

void playNote(int note, int duration) {
  int noteDuration = (60000 * 4) / (tempo * duration);
  playTone(note, noteDuration);
  delay(noteDuration * 0.1);
}

void playMelody() {
  for (int i = 0; i < noteCount; i++) {
    playNote(melody[i], durations[i]);
  }
}

void turn360() {
  // Левый мотор вперед, правый назад
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, HIGH);
  digitalWrite(LED_PIN, HIGH);
  
  delay(turnDuration); // Длительность разворота
  
  // Остановка моторов
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN3, LOW);
  digitalWrite(RIGHT_IN4, LOW);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  playMelody();       // Проигрываем мелодию
  delay(500);         // Пауза после мелодии
  turn360();          // Разворот на 360 градусов
  delay(2000);        // Пауза перед повторением
}