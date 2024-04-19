
#include <AccelStepper.h>

// Motor pin tanımları
#define motorPin1  8  // Motor 1 için IN1
#define motorPin2  9  // Motor 1 için IN2
#define motorPin3  10 // Motor 1 için IN3
#define motorPin4  11 // Motor 1 için IN4
#define motorPin5  4  // Motor 2 için IN1
#define motorPin6  5  // Motor 2 için IN2
#define motorPin7  6  // Motor 2 için IN3
#define motorPin8  7  // Motor 2 için IN4

// Stepper nesnelerinin oluşturulması
AccelStepper stepperX(AccelStepper::FULL4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);
AccelStepper stepperY(AccelStepper::FULL4WIRE, motorPin5, motorPin7, motorPin6, motorPin8);

void setup() {
  // Motor maksimum hız ve ivmelenme ayarları
  stepperX.setMaxSpeed(1000);
  stepperX.setAcceleration(500);
  stepperY.setMaxSpeed(1000);
  stepperY.setAcceleration(500);
}

void loop() {
  drawHeart();
}

void drawHeart() {
  // Kalp çizimi için adımların ayarlanması
  stepperX.moveTo(0); stepperY.moveTo(0); runMotors();
  stepperX.moveTo(250*5); stepperY.moveTo(250*5); runMotors(); // G1 X50 Y50 (Oranlı olarak büyütüldü)
  stepperX.move(500*5); stepperY.move(0); runMotors(); // G1 X100 Y50 (Oranlı olarak büyütüldü)
  stepperX.move(0); stepperY.move(-250*5); runMotors(); // G1 X100 Y0 (Oranlı olarak büyütüldü)
  stepperX.move(-500*5); stepperY.move(0); runMotors(); // G1 X50 Y0 (Oranlı olarak büyütüldü)
  stepperX.move(0); stepperY.move(250*5); runMotors(); // G1 X0 Y50 (Oranlı olarak büyütüldü)
  stepperX.moveTo(0); stepperY.moveTo(0); runMotors(); // Başlangıç noktasına dön
}

void runMotors() {
  // Motorların hedeflerine ulaşıncaya kadar çalıştırılması
  while ((stepperX.distanceToGo() != 0) || (stepperY.distanceToGo() != 0)) {
    stepperX.run();
    stepperY.run();
  }
  delay(1000); // Her hareketten sonra kısa bir duraklama
}
