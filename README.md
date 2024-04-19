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

## Gerekli Sürücüler

### Mac ve Windows Kullanıcıları için CH34x Sürücü Kurulumu

Mac işletim sistemi kullanıyorsanız ve Arduino'nuzu bilgisayarınıza bağladığınızda tanınmıyorsa, CH34x sürücüsünü yüklemeniz gerekebilir. Bu sürücü, Arduino'nun Mac ile uyumlu şekilde çalışmasını sağlar.

Sürücüyü yüklemek için aşağıdaki adımları izleyin:

1. [CH34x_Install_MAC_10_9_AND_ABOVE_V1_3.zip dosyasını indirin.
2. İndirdiğiniz `.zip` dosyasını açın ve içindeki kurulum talimatlarını takip edin.
3. Kurulum tamamlandıktan sonra bilgisayarınızı yeniden başlatın.
4. Arduino'nuzu tekrar bağlayın ve tanınıp tanınmadığını kontrol edin.

Bu sürücü yalnızca Mac OS X 10.9 ve üzeri sürümler için gereklidir. Eğer başka bir işletim sistemi kullanıyorsanız, bu adımı atlayabilirsiniz.


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
[![Video İzle](https://wearetheartmakers.com/us/images/2024/04/18/mq3.jpg)](https://youtu.be/jPZAvzi2Hsk "Videoyu İzlemek İçin Tıklayın!")

# Kalp Çizim Robotu <3 

Bu proje, Arduino ve iki adet step motor kullanarak basit bir kalp şeklini çizmek üzere tasarlanmıştır. AccelStepper kütüphanesi kullanılarak motor hareketleri kontrol edilmekte ve belirli koordinatlar doğrultusunda kalp şekli çizilmektedir.

## Donanım ve Bağlantılar

Projede iki step motor kullanılmakta ve her motor için dört kontrol pinine ihtiyaç duyulmaktadır. Motorlar, Arduino'nun dijital pinlerine bağlanır:

- **Motor 1 Pinleri:** 8, 9, 10, 11
- **Motor 2 Pinleri:** 4, 5, 6, 7

## Kurulum

Projeyi çalıştırmadan önce, aşağıdaki adımları izleyerek motorları ve Arduino'yu ayarlayın:

1. Arduino IDE'yi bilgisayarınıza kurun ve gerekli kütüphaneleri yükleyin.
2. AccelStepper kütüphanesini yükleyin ve projenin kodunu Arduino'ya yükleyin.
3. Motorları ve Arduino'yu uygun şekilde bağladığınızdan emin olun.

## Yazılım

Kod içerisinde iki `AccelStepper` nesnesi kullanılmaktadır (`stepperX` ve `stepperY`), bu nesneler motorların kontrolünü sağlamaktadır. Motorlar, FULL4WIRE modunda kullanılmaktadır.

### Fonksiyonlar

- `setup()`: Motorların maksimum hız ve ivme ayarlarını yapar.
- `drawHeart()`: Kalp şeklini çizmek için motor hareketlerini sıralar.
- `runMotors()`: Motorların hedef pozisyonlarına ulaşana kadar çalışmalarını sağlar.

## Kullanım

Arduino'yu bilgisayarınıza bağlayın ve kodu yükleyin. Arduino IDE üzerinden kodu doğrudan yükledikten sonra, cihazı güç kaynağına bağlayıp motorların bağlantılarını kontrol edin. Daha sonra sistemi başlatarak motorların kalp şeklini çizmesini izleyin.

## Geliştirme

Bu projeyi daha da geliştirmek isterseniz, motor hız ve ivmelenme ayarlarını değiştirebilir, daha karmaşık geometrik şekiller çizmek için koordinatları ayarlayabilirsiniz.



## Yardım ve Destek

Projenizle ilgili daha fazla yardıma ihtiyacınız varsa, ChatGPT ile oluşturulmuş özel yardım botumuzu kullanabilirsiniz. mDrawBot'u kolayca kurmanıza ve kullanmanıza yardımcı olacak bu bot, sorularınıza hızlı cevaplar vermek için tasarlanmıştır.

[mDrawBot Yardım Botunu Kullan](https://chat.openai.com/g/g-z5sewy5xD-watam-drawbot)

Bu hizmeti kullanabilmek için bir ChatGPT üyeliğinizin olması gerekmektedir. Eğer henüz bir üyeliğiniz yoksa, [ChatGPT kayıt sayfası](https://chat.openai.com/signup) üzerinden kolayca kayıt olabilirsiniz.


## Lisans

Bu proje MIT lisansı altında lisanslanmıştır - daha fazla bilgi için [LICENSE](LICENSE.md) dosyasına bakınız.

## İletişim

Projeyi oluşturan kişi - [watam] - admin@wearetheartmkaers.com

Proje Linki: [https://github.com/WeAreTheArtMakers/mDrawBot](https://github.com/WeAreTheArtMakers/mDrawBot)

