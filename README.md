# mDrawBot - Drawing Robot

  ![mDrawBot](https://wearetheartmakers.com/us/images/2024/04/21/EF6EC96F-E291-4013-B949-BF9D26F6A7D6.jpg)


This repository contains the source code for mDrawBot, an Arduino-based drawing robot. mDrawBot reads G-code files from an SD card and draws the specified patterns onto a surface. We are currently in the process of enhancing this project to include easy integration of an LCD screen.

## Getting Started

### Requirements

## mDrawBot Duvar Çizim Robotu - Arduino 
#### Gerekli Kütüphaneler
Kodu çalıştırmadan önce aşağıdaki kütüphanelerin Arduino IDE'ye yüklenmiş olması gerekmektedir:

- `TinyStepper_28BYJ_48`
- `Servo`
- `SD`
- `U8x8lib`
-    Bu kütüphaneleri Arduino IDE içindeki kütüphane yöneticisinden yükleyebilirsiniz.
2. Arduino IDE üzerinden `mDrawbotV3.ino` kodunu Arduino'nunuza yükleyin.


#### Donanım Bağlantısı
- İki adet `28BYJ-48` adım motoru Arduino'ya bağlanır.
- Servo motor, Arduino'nun belirli bir pinine bağlanır.
- SD kart modülü, dosya okuma için Arduino'ya entegre edilir.
- OLED ekran, kullanıcı arayüzü bilgilerini göstermek için kullanılır.

### Kullanım
Kod, Arduino üzerine yüklendikten sonra robot doğrudan kullanıma hazırdır. SD kart üzerine yüklenmiş G-kodları aracılığıyla çizim yapabilir.

### Fonksiyonlar

#### Motor Hareketleri
- `moveTo(float x, float y)`: Belirtilen x, y koordinatlarına motoru hareket ettirir.
- `line(float x, float y)`: İki nokta arasında düz bir çizgi çizer.
- `arc(float cx, float cy, float r, float start_angle, float end_angle)`: Merkezi ve yarıçapı belirtilen bir yay çizer.

#### Kalem Kontrolü
- `pen_up()`: Kalemi kaldırır.
- `pen_down()`: Kalemi indirir.

#### SD Kart İşlemleri
- `drawFile(String filename)`: SD kart üzerinden belirtilen dosya adındaki G-kodu okur ve çizimi başlatır.


### Kullanıcı Arayüzü ve Kontroller

#### OLED Ekran ve Menü Yapısı
mDrawBot, kullanıcı dostu bir OLED ekran üzerinden çeşitli işlemleri kontrol etmenize olanak tanır. OLED ekran, çizim durumu, aktif mod ve sistem bilgileri gibi önemli bilgileri gösterir. Ekran menüsü, kullanıcıların robot ayarlarını kolayca yönetebilmesi için basit ve anlaşılır bir yapıdadır.

#### Tuşlarla Kontrol
Robot, çeşitli fonksiyonlara hızlı erişim sağlayan fiziksel tuşlarla donatılmıştır. Bu tuşlar sayesinde, menüler arasında gezinebilir, çizim işlemlerini başlatabilir veya durdurabilir ve ayarları değiştirebilirsiniz. Aşağıda tuşların işlevleri ve menüde nasıl kullanılacağı detaylandırılmıştır:

- **Giriş (Enter) Tuşu**: Menü seçeneklerini onaylamak ve alt menülere girmek için kullanılır.
- **Yukarı/Aşağı Tuşları**: Menü içerisinde seçenekler arasında gezinmek için kullanılır.
- **İptal (ESC) Tuşu**: Bir önceki menüye geri dönmek veya bir işlemi iptal etmek için kullanılır.

Elbette, özür dilerim! Kodda belirtilen menü yapısını ve tuş işlevlerini aşağıdaki gibi daha doğru bir şekilde yansıtacak şekilde düzenleyelim:

### Menü İşlevleri ve Tuş Kontrolleri

mDrawBot'un kullanıcı arayüzü, OLED ekran üzerinde ve belirli tuşlar aracılığıyla çeşitli işlemleri gerçekleştirebilirsiniz. Aşağıda, kodda belirtilen menü yapısını ve her bir tuşun işlevini açıklayan detayları bulabilirsiniz:

#### Tuşlarla Kontrol
Robotun kontrolü için kullanılan tuşlar ve her birinin işlevi şunlardır:
- **Aşağı Tuşu (KEYDOWN)**: Menü seçenekleri arasında aşağı yönde hareket etmek için kullanılır.
- **Giriş Tuşu (KEYENTER)**: Seçilen menü öğesini onaylar ve alt menülere girmek için kullanılır.
- **İptal Tuşu (KEYESC)**: Bir önceki menüye dönme veya mevcut işlemi iptal etme için kullanılır.

#### Menü Yapısı
Kod içinde tanımlanan menü yapısı ve fonksiyonları şu şekildedir:

1. **Ana Menü**: 
   - Kullanıcıya çeşitli işlevler arasında seçim yapma imkanı tanır.
   - Çizim başlatma, SD karttan çizim yükleme, Set Wall / Yukarı - Aşağı Ayar değiştirme gibi seçenekler sunar.

2. **Çizim Başlatma / modART Demo Çalıştırma**:
   - Önceden tanımlanmış bir demo çizim işlemini başlatır.
   - Bu mod, robotun çizim kabiliyetlerini göstermek için idealdir.

3. **SD Karttan Çizim Yükleme**:
   - SD kart üzerindeki bir dosyayı yükler ve bu dosyada tanımlanan G-kodu çizim talimatlarına göre hareket eder.

4. **Set Wall**:
   - Robotun mevcut konumunu sıfırlar veya belirli bir konuma hareket ettirir.
   - "Up Down" konumuna dönüş veya belirlenen bir yükseklik ve genişlikte yerleşim yapma seçenekleri içerir.

Bu yapılandırmalar, kullanıcının robot üzerinde tam kontrol sahibi olmasını sağlar ve farklı çizim ihtiyaçlarına hızlı bir şekilde yanıt vermesine olanak tanır. README dosyasına bu bilgilerin eklenmesi, kullanıcıların cihazın nasıl kullanılacağını daha iyi anlamalarına yardımcı olacaktır.

### Kurulum ve Kullanım
Robotun kullanıma başlanması için gerekli adımlar ve tuş kombinasyonları, kullanıcıların cihazı hızla ve verimli bir şekilde kullanabilmeleri için detaylandırılmıştır. Kurulumdan sonra, kullanıcıların menüyü kullanarak cihaz üzerinde tam kontrol sağlamaları mümkündür.

### Konfigürasyon
Robotun çizim parametreleri ve mekanik ayarları, kod içerisindeki makrolar ve tanımlar aracılığıyla yapılandırılabilir.

### Destek
Herhangi bir sorunla karşılaşılması durumunda, [GitHub Issues](https://github.com/yourgithubusername/yourrepository/issues) üzerinden bir "issue" açarak destek isteyebilirsiniz.


### Geliştiriciler
Bu kod [WeAreTheArtMakers Studio](https://wearetheartmakers.com) tarafından geliştirilmiştir.

---

Kodu GitHub'da paylaşmak için bu README dosyasını kullanabilirsiniz. Ayrıca, kodu daha da geliştirerek ve kullanıcıların erişimine sunarak açık kaynak topluluğuna katkıda bulunabilirsiniz.
## Video Demonstrations

- **Version 1 Video:**
  [![Watch Video](https://wearetheartmakers.com/us/images/2024/04/18/mq3.jpg)](https://youtu.be/jPZAvzi2Hsk "Click to Watch!")

- **Version 2 Video:**
  [![Watch Video](https://wearetheartmakers.com/us/images/2024/04/20/MDB.webp)](https://youtu.be/U4XFW1paGl4 "Click to Watch!")

- **Version 3 Video:**
  [![Watch Video](https://wearetheartmakers.com/mdrawbot/images/mDrawBotScreen.png)](https:// "VIDEO on the way...")

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

## Katkıda Bulunma

Projeye katkıda bulunmak isterseniz:
1. Repo'yu forklayın.
2. Özellik dalınızı oluşturun (`git checkout -b feature/AmazingFeature`).
3. Değişikliklerinizi commit edin (`git commit -m 'Add some AmazingFeature'`).
4. Dalınıza push yapın (`git push origin feature/AmazingFeature`).
5. Bir Pull Request oluşturun.

## İletişim

Projeyi oluşturan kişi - [watam](mailto:admin@wearetheartmakers.com)

Proje Linki: [https://github.com/WeAreTheArtMakers/mDrawBot](https://github.com/WeAreTheArtMakers/mDrawBot)
