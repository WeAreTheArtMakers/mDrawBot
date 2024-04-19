
// Bu program, SD karttan Gcode dosyalarını okuyarak çizim yapar. Bir kart okuyucu modülüne ihtiyaç duyar.
// Bazı parametreler montaj mesafeleri ölçüldükten sonra değiştirilmelidir, doğrudan yükleme çalışmaz. Değiştirme yöntemleri için dokümana bakınız.

#include <TinyStepper_28BYJ_48.h> // Adım motoru kütüphanesi, eğer bu kütüphane yüklü değilse, Ctrl+Shift+I tuşları ile kütüphane yöneticisinden Stepper_28BYJ_48 aratıp yükleyin.
#include <Servo.h>
#include <SD.h> // SD kart okuyucu modülüne ihtiyaç vardır, eğer bu kütüphane yüklü değilse, Ctrl+Shift+I tuşları ile kütüphane yöneticisinden SD aratıp yükleyin.

// Hata ayıklama kodu işareti, yorumu kaldırarak hata ayıklama bilgileri çıktısını aktif edebilirsiniz (programın çalışma hızını yavaşlatır).
//#define VERBOSE (1)

#define STEPS_PER_TURN (2048) // Adım motorunun bir turdaki adım sayısı, 2048 adım 360 derece döner.
#define SPOOL_DIAMETER (35) // Makara çapı mm cinsinden
#define SPOOL_CIRC (SPOOL_DIAMETER * 3.1416) // Makara çevresi 35*3.14=109.956
#define TPS (SPOOL_CIRC / STEPS_PER_TURN) // Adım motoru adım mesafesi, en küçük çözünürlük, her adımda ipin çekildiği mesafe 0.053689mm

#define step_delay 1 // Adım motorunun her adımı için bekleme süresi (mikrosaniye)
#define TPD 300 // Dönüş bekleme süresi (milisaniye), eğilim nedeniyle kalem hareket etmeye devam eder, kalem durduktan sonra harekete geçmek üzere geçici olarak belirlenmiştir.


// İki motorun dönüş yönleri 1 ileri, -1 geri
#define M1_REEL_OUT 1 // İp salma
#define M1_REEL_IN -1 // İp alma
#define M2_REEL_OUT -1 // İp salma
#define M2_REEL_IN 1 // İp alma

static long laststep1, laststep2; // Mevcut ip uzunluğu, kalem pozisyonunu kaydeder

#define X_SEPARATION 507 // İki ipin üst tarafındaki yatay mesafe mm olarak
#define LIMXMAX (X_SEPARATION*0.5) // x ekseninin maksimum değeri, 0 noktası tahtanın merkezinde
#define LIMXMIN (-X_SEPARATION*0.5) // x ekseninin minimum değeri

// Dikey mesafe parametreleri: Pozitif değerler tahta altında, teorik olarak tahta yeterince büyükse sonsuz büyüklüğe kadar olabilir, negatif değerler kalem (açılış öncesi) üst tarafındadır.
#define LIMYMAX (-440) // y ekseni maksimum değeri, tahtanın en alt kısmı
#define LIMYMIN (440) // y ekseni minimum değeri, tahtanın en üst kısmı

// Kalem kaldırma servo motoru açı parametreleri, kaldıraç yerleşimine bağlı olarak ayarlanmalıdır
#define PEN_UP_ANGLE 60 // Kalem kaldırma
#define PEN_DOWN_ANGLE 95 // Kalem indirme

#define PEN_DOWN 1 // Kalem durumu aşağı
#define PEN_UP 0 // Kalem durumu yukarı


// Nokta yapısını tanımlar, üç boyutlu koordinat sistemi için kullanılır.
struct point {
  float x;  // X koordinatı
  float y;  // Y koordinatı
  float z;  // Z koordinatı, genellikle kalem yüksekliği için kullanılır
};

struct point actuatorPos;  // Aktüatörün (motorun hareket ettirdiği parçanın) pozisyonunu tutar.

// Çizim robotunun pozisyon değişkenleri
static float posx;  // X pozisyonu
static float posy;  // Y pozisyonu
static float posz;  // Kalem durumu (yukarı veya aşağı)

static float feed_rate = 0;  // Besleme oranı, çizim hızını kontrol edebilir

// Kalem durumu için değişken; kalem yukarıda veya aşağıda olabilir.
static int ps;

// G kodu iletişim parametreleri
#define BAUD            (115200)    // Seri bağlantı hızı, G kodu iletimi veya hata ayıklama için kullanılır. Alternatif olarak 9600, 57600, 115200 kullanılabilir.
#define MAX_BUF         (64)        // Seri tampon alanı büyüklüğü

// Seri iletişim alımı
static int sofar;               // Şimdiye kadar alınan seri veri miktarı

static float mode_scale;   // Ölçeklendirme faktörü, çizim ölçeğini ayarlamak için kullanılır

File myFile;  // Dosya işlemleri için File nesnesi

Servo pen;  // Kalem servo motoru

// Adım motorları tanımlamaları
TinyStepper_28BYJ_48 m1; //(7,8,9,10);  // M1 adım motoru, Arduino UNO'nun 7, 8, 9, 10 pinlerine bağlı
TinyStepper_28BYJ_48 m2; //(2,3,5,6);  // M2 adım motoru, Arduino UNO'nun 2, 3, 5, 6 pinlerine bağlı

//------------------------------------------------------------------------------
// İleri kinematik hesaplama - L1 ve L2 uzunluklarını X ve Y koordinatlarına dönüştürür
// Kosinüs teoremi kullanılarak theta açısı hesaplanır: theta = acos((a*a + b*b - c*c) / (2*a*b))
// Burada M1M2 motorlar arası mesafe ve M1P noktası arasındaki açı hesaplanır, P kalem pozisyonudur
void FK(float l1, float l2, float &x, float &y) {
  float a = l1 * TPS;  // L1'i adım uzunluğuna çevirir
  float b = X_SEPARATION;  // Motorlar arası mesafe
  float c = l2 * TPS;  // L2'yi adım uzunluğuna çevirir

  // Açıyı hesapla ve X, Y koordinatlarını ayarla
  float theta = acos((a * a + b * b - c * c) / (2.0 * a * b));
  x = cos(theta) * l1 + LIMXMIN;  // X koordinatını hesapla
  y = sin(theta) * l1 + LIMYMIN;  // Y koordinatını hesapla
}

//------------------------------------------------------------------------------
// Ters kinematik hesaplama - XY koordinatlarını L1 ve L2 uzunluklarına dönüştürür
void IK(float x, float y, long &l1, long &l2) {
  float dy = y - LIMYMIN;  // Mevcut y'den minimum y'yi çıkar
  float dx = x - LIMXMIN;  // Mevcut x'den minimum x'i çıkar
  l1 = round(sqrt(dx * dx + dy * dy) / TPS);  // L1 uzunluğunu hesapla
  dx = x - LIMXMAX;
  l2 = round(sqrt(dx * dx + dy * dy) / TPS);  // L2 uzunluğunu hesapla
}

//------------------------------------------------------------------------------
// Kalem durumu fonksiyonu - Kalem aşağı veya yukarı hareket ettirilir
void pen_state(int pen_st) {
  if (pen_st == PEN_DOWN) {
    ps = PEN_DOWN_ANGLE;  // Kalem aşağı açısı
    pen.write(ps);
  } else {
    ps = PEN_UP_ANGLE;  // Kalem yukarı açısı
    pen.write(ps);
  }
}

//------------------------------------------------------------------------------
// Kalem aşağı indirme fonksiyonu
void pen_down() {
  if (ps == PEN_UP_ANGLE) {
    ps = PEN_DOWN_ANGLE;  // Kalem aşağı açısını ayarla
    pen.write(ps);  // Servoya yaz
    delay(TPD);  // Belirli bir süre bekle
  }
}

//------------------------------------------------------------------------------
// Kalem yukarı kaldırma fonksiyonu
void pen_up() {
  if (ps == PEN_DOWN_ANGLE) {
    ps = PEN_UP_ANGLE;  // Kalem yukarı açısını ayarla
    pen.write(ps);  // Servoya yaz
  }
}

//------------------------------------------------------------------------------
// Mevcut makine durumunu seri port üzerinden raporla
void where() {
  Serial.print("X,Y= ");
  Serial.print(posx);
  Serial.print(",");
  Serial.print(posy);
  Serial.print("\tLst1,Lst2= ");
  Serial.print(laststep1);
  Serial.print(",");
  Serial.println(laststep2);
}

//------------------------------------------------------------------------------
// Verilen x ve y farklarından açıyı 0...2PI aralığında hesaplar
static float atan3(float dy, float dx) {
  float a = atan2(dy, dx);
  if (a < 0) a += (PI * 2.0);  // Negatif açıları düzelt
  return a;
}

//------------------------------------------------------------------------------
// Yay çizimi için kullanılan fonksiyon
static void arc(float cx, float cy, float x, float y, float dir) {
  // Yarıçapı hesapla
  float dx = posx - cx;
  float dy = posy - cy;
  float radius = sqrt(dx * dx + dy * dy);

  // Yayın başlangıç ve bitiş açılarını bul
  float angle1 = atan3(dy, dx);
  float angle2 = atan3(y - cy, x - cx);
  float theta = angle2 - angle1;

  // Açı düzeltmesi yap
  if (dir > 0 && theta < 0) angle2 += 2 * PI;
  else if (dir < 0 && theta > 0) angle1 += 2 * PI;

  // Yay uzunluğunu hesapla
  float len = abs(theta) * radius;
  int i, segments = floor(len / TPS);

  float nx, ny, angle3, scale;

  // Yayı segmentlere böl ve her segment için çizim yap
  for (i = 0; i < segments; ++i) {
    if (i == 0)
      pen_up();
    else
      pen_down();
    scale = ((float)i) / ((float)segments);
    angle3 = (theta * scale) + angle1;
    nx = cx + cos(angle3) * radius;
    ny = cy + sin(angle3) * radius;
    line_safe(nx, ny);
  }

  line_safe(x, y);
  pen_up();
}

//------------------------------------------------------------------------------
// Sanal çizim pozisyonunu anında taşıma
// Hareketin geçerli olup olmadığı doğrulanmaz
static void teleport(float x, float y) {
  posx = x;  // Yeni x pozisyonunu kaydet
  posy = y;  // Yeni y pozisyonunu kaydet
  long l1, l2;
  IK(posx, posy, l1, l2);  // İnvers kinematik hesaplamalarını yap
  laststep1 = l1;  // Son adım değerlerini güncelle
  laststep2 = l2;
}

//==========================================================
// Dikey hareketler için kullanılan fonksiyon
void moveto(float x, float y) {
  #ifdef VERBOSE
  // Harekete başlama bilgisini seri porta yazdır
  Serial.println("Hareket fonksiyonuna girildi");
  Serial.print("x:");
  Serial.print(x);
  Serial.print(" y:");
  Serial.println(y);
  #endif

  long l1, l2;
  IK(x, y, l1, l2); // İstenen x, y koordinatlarına göre l1, l2 uzunluklarını hesapla
  long d1 = l1 - laststep1; // Mevcut konumdan yeni konuma olan farkı hesapla
  long d2 = l2 - laststep2;

  #ifdef VERBOSE
  // Hesaplanan değerleri seri porta yazdır
  Serial.print("l1:");
  Serial.print(l1);
  Serial.print(" laststep1:");
  Serial.print(laststep1);
  Serial.print(" d1:");
  Serial.println(d1);
  Serial.print("l2:");
  Serial.print(l2);
  Serial.print(" laststep2:");
  Serial.print(laststep2);
  Serial.print(" d2:");
  Serial.println(d2);
  #endif

  long ad1 = abs(d1); // Farkların mutlak değerini al
  long ad2 = abs(d2);
  int dir1 = d1 > 0 ? M1_REEL_IN : M1_REEL_OUT; // Yön belirleme
  int dir2 = d2 > 0 ? M2_REEL_IN : M2_REEL_OUT;
  long over = 0;
  long i;

  // Eğer bir eksendeki adım diğer eksendeki adımdan fazla ise
  if (ad1 > ad2) {
    for (i = 0; i < ad1; ++i) {
      m1.moveRelativeInSteps(dir1); // M1 motorunu hareket ettir
      over += ad2;
      if (over >= ad1) {
        over -= ad1;
        m2.moveRelativeInSteps(dir2); // M2 motorunu hareket ettir
      }
      delayMicroseconds(step_delay); // Belirlenen süre kadar bekle
    }
  } 
  else {
    for (i = 0; i < ad2; ++i) {
      m2.moveRelativeInSteps(dir2);
      over += ad1;
      if (over >= ad2) {
        over -= ad2;
        m1.moveRelativeInSteps(dir1);
      }
      delayMicroseconds(step_delay);
    }
  }

  laststep1 = l1; // Son adım değerlerini güncelle
  laststep2 = l2;
  posx = x; // Yeni pozisyonu kaydet
  posy = y;
}

//------------------------------------------------------------------------------
// Uzun mesafeli hareketlerde çizilen çizginin yörüngesi bir yay şeklini alabilir,
// bu nedenle uzun çizgileri daha düzgün hale getirmek için kısa çizgilere bölünür.
static void line_safe(float x, float y) {
  float dx = x - posx; // Mevcut x pozisyonundan hedef x pozisyonuna olan fark
  float dy = y - posy; // Mevcut y pozisyonundan hedef y pozisyonuna olan fark

  float len = sqrt(dx * dx + dy * dy); // Başlangıç noktasından hedef noktasına olan uzaklık
  
  if (len <= TPS) { // Eğer uzunluk adım motorunun tek adımda aldığı mesafeden küçük veya eşitse
    moveto(x, y); // Direkt hedef noktasına hareket et
    return;
  }
  
  // Çok uzun bir mesafe!
  long pieces = floor(len / TPS); // Uzunluğu adım motorunun adım mesafesine böl ve kaç parça gerektiğini hesapla
  float x0 = posx;
  float y0 = posy;
  float a;
  for (long j = 0; j <= pieces; ++j) {
    a = (float)j / (float)pieces; // Oransal bir katsayı hesapla

    // Başlangıç noktasından itibaren orantılı bir şekilde hedef noktaya doğru hareket et
    moveto((x - x0) * a + x0, (y - y0) * a + y0);
  }
  moveto(x, y); // Son noktaya hareketi garanti altına al
}

void line(float x, float y) {
  line_safe(x, y); // Güvenli çizgi çizme fonksiyonunu çağırarak, verilen koordinatlara çizgi çiz
}


//********************************
void nc(String st) {
  String xx, yy, zz; // X, Y ve Z koordinatlarını tutacak string değişkenleri
  int ok = 1; // Koordinatların doğru şekilde ayrıştırılıp ayrıştırılmadığını kontrol eden değişken

  st.toUpperCase(); // Gelen stringi büyük harfe çevir
  
  float x, y, z; // Koordinatları tutacak float değişkenler
  int px, py, pz; // 'X', 'Y', ve 'Z' harflerinin pozisyonlarını tutacak değişkenler
  px = st.indexOf('X'); // 'X' harfinin pozisyonunu bul
  py = st.indexOf('Y'); // 'Y' harfinin pozisyonunu bul
  pz = st.indexOf('Z'); // 'Z' harfinin pozisyonunu bul
  
  if (px == -1 || py == -1) ok = 0; // Eğer 'X' veya 'Y' bulunamazsa hata ver
  if (pz == -1) {
    pz = st.length(); // Eğer 'Z' bulunamazsa, 'Z' pozisyonunu stringin sonu olarak ayarla
  } else {   
    zz = st.substring(pz + 1); // 'Z' den sonra gelen koordinatı al
    z = zz.toFloat(); // Stringi float'a çevir
    if (z > 0) pen_up(); // Eğer z pozitifse kalemi kaldır
    if (z <= 0) pen_down(); // Eğer z negatif veya sıfırsa kalemi indir
  }

  xx = st.substring(px + 1, py); // 'X' ile 'Y' arasındaki değeri al
  yy = st.substring(py + 1, pz); // 'Y' ile 'Z' (veya son) arasındaki değeri al
  
  xx.trim(); // 'xx' değişkeninin başındaki ve sonundaki boşlukları sil
  yy.trim(); // 'yy' değişkeninin başındaki ve sonundaki boşlukları sil

  if (ok) line(xx.toFloat(), yy.toFloat()); // Eğer hata yoksa elde edilen x ve y koordinatları ile çizgi çiz
}


//**********************
void drawfile(String filename) {
  String rd = "";  // Okunan veriyi saklamak için string değişkeni
  int line = 0;    // Satır sayacı
  char rr = 0;     // Karakter okuma değişkeni
  Serial.print("[");
  Serial.print(filename);  // Dosya adını seri porttan gönder
  
  myFile = SD.open(filename);  // SD karttan dosya aç
  
  if (myFile) {  // Dosya başarıyla açıldıysa
    Serial.println("] Açıldı");
    
    while (myFile.available()) {  // Dosyadan okunacak veri kaldıkça
      rr = myFile.read();  // Bir karakter oku
      
      if (rr == char(10)) {  // Eğer okunan karakter yeni satır karakteri ise
        line++;  // Satır sayısını arttır
        Serial.print("Çalıştır nc #");
        Serial.print(line);  // Kaçıncı satır olduğunu yazdır
        Serial.println(" : " + rd);  // Okunan satırı yazdır
        nc(rd);  // Okunan satırı işle
        rd = "";  // Okuma string'ini sıfırla
      } else {
        rd += rr;  // Yeni satır karakteri değilse, okunan karakteri string'e ekle
      }        
    }
    
    myFile.close();  // Dosyayı kapat
    
  } else {  // Dosya açılamadıysa
    Serial.println("Dosya açma hatası.");  // Hata mesajı göster
  }
}


void setup() {
  // Kurulum kodunuzu buraya yazın, bu kod sadece bir kez çalıştırılır:
  Serial.begin(BAUD);  // Seri iletişim başlatılıyor, BAUD hızında.
  
  m1.connectToPins(7,8,9,10); // M1 L adım motoru için UNO'nun 7, 8, 9, 10 numaralı pinlerine bağlanıyor
  m2.connectToPins(2,3,5,6);  // M2 R adım motoru için UNO'nun 2, 3, 5, 6 numaralı pinlerine bağlanıyor
  m1.setSpeedInStepsPerSecond(10000);  // M1 motorunun saniyede adım hızı ayarlanıyor
  m1.setAccelerationInStepsPerSecondPerSecond(100000);  // M1 motorunun ivmelenmesi ayarlanıyor
  m2.setSpeedInStepsPerSecond(10000);  // M2 motorunun saniyede adım hızı ayarlanıyor
  m2.setAccelerationInStepsPerSecondPerSecond(100000);  // M2 motorunun ivmelenmesi ayarlanıyor

  // Kalem kaldırma için servo motor bağlanıyor
  pen.attach(A0);  // Servo motor A0 pinine bağlanıyor
  ps = PEN_UP_ANGLE;  // Kalem kaldırma açısı ayarlanıyor
  pen.write(ps);  // Servo motor kalem kaldırma açısına ayarlanıyor

  // Kalem pozisyonu (0, 0) olarak ayarlanıyor
  teleport(0, 0);
  // Ölçeklendirme oranı ayarlanıyor
  mode_scale = 1;

  // SD kart başlatılıyor, başarısız olursa hata mesajı yazdırılıyor ve döngüye giriliyor
  if (!SD.begin(4)) {
    Serial.println("SD kart başlatma başarısız!");
    while (1);  // Hata durumunda sonsuz döngüye giriliyor
  }

  // Kurulum başarılı mesajı yazdırılıyor
  Serial.println("Test OK!");
}


void loop() {
  // Dikkat edin, karttaki dosya adı programdaki ile aynı olmalı!
  
  drawfile("1.nc");  // "1.nc" G-code dosyasının adıdır, G-code'u SD kartta kaydetmeniz gerekir.
  // Apple sistemlerinde, dosya adını 1.txt gibi değiştirin, çünkü NC dosyasını kopyalarken sistem dosya adını değiştirebilir.
  while(1);  // Sonsuz döngü
}

