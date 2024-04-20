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
