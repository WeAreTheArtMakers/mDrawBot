# Çizim Robotu Projesi

Bu depo, Arduino tabanlı bir çizim robotunun kaynak kodlarını içermektedir. Robot, G-code dosyalarını SD karttan okuyarak belirlenen desenleri bir yüzeye çizer.

## Başlarken

Bu bölümde, projeyi lokal ortamınıza klonlama ve çalıştırma adımları yer almaktadır.

### Önkoşullar

Bu projeyi çalıştırmak için aşağıdaki malzemelere ihtiyacınız vardır:
- Arduino UNO
- İki adet TinyStepper_28BYJ_48 adım motoru
- SD kart modülü
- Servo motor
- Gerekli bağlantı kabloları ve güç kaynağı
- (Opsiyonel) LCD ekran ve kontrol butonları

### Kurulum

Projeyi kendi Arduino ortamınıza kurmak için aşağıdaki adımları izleyin:

1. Arduino IDE'yi bilgisayarınıza kurun.
2. Bu repo'yu klonlayın veya indirin.
3. `TinyStepper_28BYJ_48` ve `SD` kütüphanelerini Arduino IDE'ye yükleyin. Kütüphaneleri yüklemek için, Arduino IDE'de `Sketch > Include Library > Manage Libraries` menüsünü kullanabilirsiniz.
4. Arduino'yu bilgisayarınıza bağlayın.
5. İndirdiğiniz proje dosyasını Arduino IDE'ye yükleyin ve ardından `Upload` düğmesine basarak kodu Arduino'ya yükleyin.

### Kullanım

Robotun çalışması için, G-code formatında çizim komutlarını içeren bir `.nc` dosyasını SD karta yükleyin ve Arduino'ya takın. Sistem otomatik olarak SD kartı okuyacak ve çizimi yapacaktır.

## Konfigürasyon

Robotun çeşitli parametreleri, kod içindeki `#define` ifadeleri aracılığıyla ayarlanabilir. Bu parametreler arasında motor hızı, kalem açıları ve adım mesafesi bulunmaktadır.

## Katkıda Bulunma

Projeye katkıda bulunmak istiyorsanız, lütfen şunları yapın:
1. Repo'yu forklayın.
2. Feature branch'ınızı oluşturun (`git checkout -b feature/AmazingFeature`).
3. Değişikliklerinizi commit edin (`git commit -m 'Add some AmazingFeature'`).
4. Branch'ınıza push yapın (`git push origin feature/AmazingFeature`).
5. Bir Pull Request oluşturun.

Versiyon 1 Video
[![MDRAW BOT V1 Video]([http://img.youtube.com/vi/YOUTUBE_VIDEO_ID/maxresdefault.jpg](https://i9.ytimg.com/vi/jPZAvzi2Hsk/mq3.jpg))](http://www.youtube.com/watch?v=jPZAvzi2Hsk "MDRAW BOT V1 Video")


## Lisans

Bu proje MIT lisansı altında lisanslanmıştır - daha fazla bilgi için [LICENSE](LICENSE.md) dosyasına bakınız.

## İletişim

Projeyi oluşturan kişi - [watam] - admin@wearetheartmkaers.com

Proje Linki: [https://github.com/WeAreTheArtMakers/mDrawBot](https://github.com/WeAreTheArtMakers/mDrawBot)

