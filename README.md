# mDrawBot - Drawing Robot

This repository contains the source code for mDrawBot, an Arduino-based drawing robot. mDrawBot reads G-code files from an SD card and draws the specified patterns onto a surface. We are currently in the process of enhancing this project to include easy integration of an LCD screen.

## Getting Started

### Requirements

- Arduino UNO
- Stepper Motors
- Servo Motor
- SD Card Reader Module
- LCD Screen (integration in development)

### Setup

1. Install the required libraries:
   - `TinyStepper_28BYJ_48`
   - `Servo`
   - `SD`
   You can install these libraries automatically through the Arduino IDE.
2. Load the `mDrawbot.ino` code onto your Arduino using the Arduino IDE.

### Usage

To use the device, load a Gcode formatted file onto your SD card:
- For Windows computers, use `1.nc`
- For MacOS operating systems, use `1.txt`

When you insert the SD card into the Arduino, mDrawBot will automatically start drawing. If you remove the SD card during drawing, the drawing will stop.

## Video Demonstrations

- **Version 1 Video:**
  [![Watch Video](https://wearetheartmakers.com/us/images/2024/04/18/mq3.jpg)](https://youtu.be/jPZAvzi2Hsk "Click to Watch!")

- **Version 2 Video:**
  [![Watch Video](https://wearetheartmakers.com/us/images/2024/04/20/MDB.webp)](https://youtu.be/U4XFW1paGl4 "Click to Watch!")

## Key Functions in mDrawBot.ino

### `setup()`
This function runs once at the start of the Arduino. It sets up pin definitions and initializes the motors to their starting positions.

### `loop()`
This is the main loop of the Arduino. Inside this function, motor movements are controlled according to the G-code commands read from the SD card, and the drawing process is carried out.

### `readGCode()`
Reads a line of G-code from the SD card and processes it. This function determines the positions and speeds at which the motors should move.

### `moveStepper()`
Moves the stepper motors according to the read G-code. It determines the direction and number of steps the motors should take.

### `penUp()` and `penDown()`
Uses the servo motor to lift and lower the pen. Used during drawing and transitions between drawings.

## Contributing

We are seeking contributors for the development of easy integration of an LCD screen. If you would like to contribute to this or other aspects of the project:
1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/LCDEnhancement` or `feature/AmazingFeature`).
3. Commit your changes (`git commit -m 'Add LCDEnhancement'` or `git commit -m 'Add some AmazingFeature'`).
4. Push to the branch (`git push origin feature/LCDEnhancement` or `feature/AmazingFeature`).
5. Create a Pull Request.

## License

This project is licensed under the MIT License. For details, see the [LICENSE](LICENSE.md) file.

## Contact

Project Creator - [watam](mailto:admin@wearetheartmakers.com)

Project Link: [https://github.com/WeAreTheArtMakers/mDrawBot](https://github.com/WeAreTheArtMakers/mDrawBot)


# mDrawBot - Çizim Robotu 

Bu depo, Arduino tabanlı bir çizim robotu olan mDrawBot'un kaynak kodlarını içermektedir. mDrawBot, G-code dosyalarını SD karttan okuyarak belirlenen desenleri bir yüzeye çizer.

## Başlarken

### Gereksinimler

- Arduino UNO
- Stepper Motorlar
- Servo Motor
- SD Kart Okuyucu Modülü

### Kurulum

1. Gerekli kütüphaneleri yükleyin:
   - TinyStepper_28BYJ_48
   - Servo
   - SD
   Bu kütüphaneleri Arduino IDE'den otomatik olarak yükleyebilirsiniz.
2. Arduino IDE üzerinden `mDrawbot.ino` kodunu Arduino'nunuza yükleyin.

### Kullanım

Cihazı kullanmak için, SD kartınıza Gcode formatındaki dosyayı yükleyin:
- Windows bilgisayarlar için `1.nc`
- MacOS işletim sistemleri için `1.txt`

SD kartı Arduino'ya taktığınızda, mDrawBot otomatik olarak çizmeye başlar. Çizim sırasında SD kartı çıkartırsanız, çizim durur.

## Video Gösterimi

- **Versiyon 1 Video:**
  [![Video İzle](https://wearetheartmakers.com/us/images/2024/04/18/mq3.jpg)](https://youtu.be/jPZAvzi2Hsk "Videoyu İzlemek İçin Tıklayın!")

- **Versiyon 2 Video:**
  [![Video İzle](https://wearetheartmakers.com/us/images/2024/04/20/MDB.webp)](https://youtu.be/U4XFW1paGl4 "Videoyu İzlemek İçin Tıklayın!")

## mDrawBot.ino'da Önemli Fonksiyonlar

### `setup()`
Arduino'nun başlangıcında bir kez çalışır. Bu fonksiyon, pin tanımlamalarını yapar ve motorları başlangıç pozisyonlarına getirir.

### `loop()`
Arduino'nun ana döngüsüdür. Bu fonksiyon içinde, SD karttan okunan G-code komutlarına göre motor hareketleri yönetilir ve çizim işlemi gerçekleştirilir.

### `readGCode()`
SD karttan bir satır G-code okur ve işler. Bu fonksiyon, motorların hareket etmesi gereken pozisyonları ve hızları belirler.

### `moveStepper()`
Okunan G-code'a göre stepper motorlarını hareket ettirir. Motorların hangi yönde ve ne kadar adım atacağını belirler.

### `penUp()` ve `penDown()`
Servo motoru kullanarak kalemi kaldırır ve indirir. Çizim yapılırken ve çizim arası geçişlerde kullanılır.

## Katkıda Bulunma

Projeye katkıda bulunmak isterseniz:
1. Repo'yu forklayın.
2. Özellik dalınızı oluşturun (`git checkout -b feature/AmazingFeature`).
3. Değişikliklerinizi commit edin (`git commit -m 'Add some AmazingFeature'`).
4. Dalınıza push yapın (`git push origin feature/AmazingFeature`).
5. Bir Pull Request oluşturun.

## Lisans

Bu proje MIT lisansı altında lisanslanmıştır. Detaylar için [LICENSE](LICENSE.md) dosyasına bakınız.

## İletişim

Projeyi oluşturan kişi - [watam](mailto:admin@wearetheartmakers.com)

Proje Linki: [https://github.com/WeAreTheArtMakers/mDrawBot](https://github.com/WeAreTheArtMakers/mDrawBot)
