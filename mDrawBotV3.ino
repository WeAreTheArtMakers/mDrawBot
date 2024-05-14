// mDrawBot Duvar Çizim Robotu Kodları
// WATAM - @STUDIOBRN WeAreTheArtMakers.com 
// OPEN-SOURCE modARTCollective 

#include <TinyStepper_28BYJ_48.h> // Adım motoru için kütüphane, eğer bu kütüphane yüklü değilse Ctrl+Shift+I ile kütüphane yöneticisinden Stepper_28BYJ_48 aratın ve yükleyin.
#include <Servo.h>
#include <SD.h> // SD kart kütüphanesi, eğer bu kütüphane yüklü değilse Ctrl+Shift+I ile kütüphane yöneticisinden SD aratın ve yükleyin.
#include <U8x8lib.h> // U8G2 kütüphanesi, eğer bu kütüphane yüklü değilse Ctrl+Shift+I ile kütüphane yöneticisinden u8g2 aratın ve yükleyin.

#define STEPS_PER_TURN (2048) // Adım motorunun bir dönüşteki adım sayısı, 2048 adım 360 derece dönüş.
#define SPOOL_DIAMETER (35) // Makara çapı mm cinsinden
#define SPOOL_CIRC (SPOOL_DIAMETER * 3.1416) // Makara çevresi 35*3.14=109.956
#define THREAD_PER_STEP (SPOOL_CIRC / STEPS_PER_TURN) // Adım motorunun adım başına düşen çözünürlüğü, yani her adımda çekilen ipin uzunluğu 0.053689mm.

#define step_delay 1 // Adım motorunun her adımı arasındaki bekleme süresi (mikrosaniye).
#define TPD 300 // Dönüş bekleme süresi (milisaniye), çünkü inerseyel hareket nedeniyle kalem hareket etmeye devam eder, geçici olarak kalemin durmasını bekleyin.

// İki motorun dönüş yönü, 1 ileri, -1 geri.
// Yön ayarlarını değiştirerek görüntüyü dikey olarak çevirebilirsiniz.
#define M1_REEL_OUT -1 // İp salma
#define M1_REEL_IN 1 // İp toplama
#define M2_REEL_OUT 1 // İp salma
#define M2_REEL_IN -1 // İp toplama

static long laststep1, laststep2; // Geçerli ip uzunluğu, kalem pozisyonunu kaydeder.

#define X_SEPARATION 400 // İki ipin üst kısmındaki yatay mesafe mm olarak.
#define LIMXMAX (X_SEPARATION*0.5) // x ekseninin maksimum sınırı, 0 noktası tahtanın ortası.
#define LIMXMIN (-X_SEPARATION*0.5) // x ekseninin minimum sınırı.

// Dikey mesafe parametreleri: Pozitif değerler tahtanın altında, teorik olarak tahta yeterince büyükse sonsuza kadar büyüyebilir, negatif değerler kalem (başlangıçta) üstünde.
#define LIMYMAX (-250) // y ekseninin maksimum sınırı, tahtanın en altı.
#define LIMYMIN (250) // y ekseninin minimum sınırı, tahtanın en üstü. Sol ve sağ iplerin sabit noktalarından kaleme olan dikey mesafe, mümkün olduğunca doğru ölçmeye çalışın, büyük hata bozulmaya neden olur.
                             // Değer küçültüldüğünde çizim ince uzun, değer büyütüldüğünde çizim kısa kalın olur.

// Kalem kaldırma servo motorunun açı parametreleri, somunun yerleştirildiği pozisyona göre ayarlanmalıdır.
#define PEN_UP_ANGLE 105 // Kalem kaldırma
#define PEN_DOWN_ANGLE 50 // Kalem indirme
// Ayarlanması gereken parametreler =============================================

#define PEN_DOWN 1 // Kalem durumu, aşağıda
#define PEN_UP 0 // Kalem durumu, yukarıda


//U8X8 SSD1306 128x64 boyutunda bir OLED ekranı başlatır. Bu ekran I2C protokolü üzerinden iletişim kurar.
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE); 

// Nokta yapısı, x, y ve z koordinatlarını tutar. 
struct point { 
  float x; // x koordinatı
  float y; // y koordinatı
  float z; // z koordinatı (genellikle z ekseni kalem yukarı veya aşağı durumu için kullanılır)
};

struct point actuatorPos; // Aktüatörün (hareket mekanizmasının) pozisyonunu tutar.

// Plotter'ın (çizim robotunun) pozisyonunu tutan değişkenler.
static float posx; // x pozisyonu
static float posy; // y pozisyonu
static float posz; // z pozisyonu, kalem durumu (yukarı veya aşağı)
static float feed_rate = 0; // Besleme oranı, çizim hızını kontrol eder.

// Kalem durumu, PEN_UP veya PEN_DOWN makroları ile belirlenir.
static int ps; // Kalem durumu

// Aşağıdaki tanımlar, G-kodu iletişimi için seri port parametrelerini belirler.
#define BAUD            (115200)    // Seri iletişim hızı, G-kodu iletişimi ve hata ayıklama için kullanılır.
#define MAX_BUF         (64)        // Seri alım tamponunun büyüklüğü

// Seri iletişimde alınan veri miktarını izler.
static int sofar;               // Şu ana kadar alınan veri miktarı

static float mode_scale;   // Oranlama faktörü, koordinatların ölçeklendirilmesi için kullanılır.

File myFile; // SD karttan dosya okumak için kullanılan File nesnesi

Servo pen; // Kalem hareketini kontrol eden servo motor
#define BEEP_SW 1 // İş tamamlandığında bip sesi çalıp çalmayacağını belirler (1 çalar, 0 çalmaz)
#define BEEP A2  // Buzzer'ın bağlı olduğu pin
#define ADKEY A1 // Analog tuş takımının bağlı olduğu pin
#define KEYDOWN 330 // Tuşa basılma değeri
#define KEYENTER 90 // "Enter" tuşunun analog değeri
#define KEYESC 517 // "Escape" tuşunun analog değeri

// İki adet TinyStepper_28BYJ_48 motoru tanımlanır. Bu motorlar çizim robotunun hareketini sağlar.
TinyStepper_28BYJ_48 m1; //(7,8,9,10);  // M1 L adım motoru, UNO'nun 7, 8, 9, 10 pinlerine bağlı
TinyStepper_28BYJ_48 m2; //(2,3,5,6);  // M2 R adım motoru, UNO'nun 2, 3, 5, 6 pinlerine bağlı


//------------------------------------------------------------------------------
// Pozitif yönde hareket hesaplama - L1 ve L2 uzunluklarını XY koordinatlarına çevirir.
// Kosinüs teoremi kullanarak theta açısını hesaplar: theta = acos((a*a + b*b - c*c) / (2*a*b));
// Bu, M1 ve M2 motorları arasındaki ve M1 ile kalem pozisyonu P arasındaki açıyı bulur.
void FK(float l1, float l2, float &x, float &y) {
  float a = l1 * THREAD_PER_STEP;  // L1 uzunluğunu adım başına düşen çekme mesafesi ile çarpar
  float b = X_SEPARATION;  // Motorlar arası X ekseni üzerindeki mesafe
  float c = l2 * THREAD_PER_STEP;  // L2 uzunluğunu adım başına düşen çekme mesafesi ile çarpar

  // Yöntem 1:
  float theta = acos((a*a + b*b - c*c) / (2.0 * a * b));  // Kosinüs teoremi ile theta açısını hesapla
  x = cos(theta) * l1 + LIMXMIN;  // X koordinatını hesapla
  y = sin(theta) * l1 + LIMYMIN;  // Y koordinatını hesapla

  // Yöntem 2: (Yorum satırlarındaki alternatif yöntem)
  // float theta = (a*a + b*b - c*c) / (2.0 * a * b);
  // x = theta * l1 + LIMXMIN;
  // y = sqrt(1.0 - theta * theta) * l1 + LIMYMIN;
}

//------------------------------------------------------------------------------
// Ters yönde hareket - XY koordinatlarını uzunluklara L1 ve L2'ye çevirir.
void IK(float x, float y, long &l1, long &l2) {
  float dy = y - LIMYMIN;  // Y ekseni üzerindeki pozisyon farkı
  float dx = x - LIMXMIN;  // X ekseni üzerindeki pozisyon farkı
  l1 = round(sqrt(dx*dx + dy*dy) / THREAD_PER_STEP);  // dx ve dy kullanarak L1'i hesapla
  dx = x - LIMXMAX;  // X maksimum sınırdan uzaklık
  l2 = round(sqrt(dx*dx + dy*dy) / THREAD_PER_STEP);  // L2'yi hesapla
}

//------------------------------------------------------------------------------
// Kalem durumunu ayarlar: PEN_UP veya PEN_DOWN.
void pen_state(int pen_st) {
  if (pen_st == PEN_DOWN) {
    ps = PEN_DOWN_ANGLE;  // Kalem aşağı konumda
  } else {
    ps = PEN_UP_ANGLE;    // Kalem yukarı konumda
  }
  pen.write(ps);  // Servo motoru uygun açıya döndür
}

//------------------------------------------------------------------------------
// Kalem aşağı fonksiyonu
void pen_down() {
  if (ps == PEN_UP_ANGLE) {
    ps = PEN_DOWN_ANGLE;  // Kalem durumunu aşağıya çevir
    pen.write(ps);        // Servo motoru uygun açıya döndür
    delay(TPD);           // Tanımlanan gecikme süresi kadar bekle
  }
}

//------------------------------------------------------------------------------
// Kalem yukarı fonksiyonu
void pen_up() {
  if (ps == PEN_DOWN_ANGLE) {
    ps = PEN_UP_ANGLE;    // Kalem durumunu yukarıya çevir
    pen.write(ps);        // Servo motoru uygun açıya döndür
  }
}

//------------------------------------------------------------------------------
// dy/dx için açıyı 0 ile 2PI arasında bir değer olarak döndürür.
static float atan3(float dy, float dx) {
  float a = atan2(dy, dx); // Arctangent2 fonksiyonu, dx ve dy arasındaki açıyı radian cinsinden hesaplar.
  if (a < 0) a = (PI * 2.0) + a; // Eğer açı negatif ise, 2PI ekleyerek pozitif hale getirir.
  return a; // Hesaplanan açıyı döndürür.
}

//------------------------------------------------------------------------------
// Belirtilen merkez etrafında bir yay çizer.
static void arc(float cx, float cy, float x, float y, float dir) {
  // Yarıçapı hesapla
  float dx = posx - cx;
  float dy = posy - cy;
  float radius = sqrt(dx * dx + dy * dy); // Merkez ile mevcut nokta arasındaki mesafeyi yarıçap olarak hesapla.

  // Yayın açısını bul (sweep)
  float angle1 = atan3(dy, dx); // Başlangıç açısını hesapla.
  float angle2 = atan3(y - cy, x - cx); // Bitiş açısını hesapla.
  float theta = angle2 - angle1; // Toplam yay açısını hesapla.

  // Yay yönünü düzelt
  if (dir > 0 && theta < 0) angle2 += 2 * PI; // Pozitif yön ve negatif açı için açıyı düzelt.
  else if (dir < 0 && theta > 0) angle1 += 2 * PI; // Negatif yön ve pozitif açı için açıyı düzelt.

  // Yayın uzunluğunu hesapla
  float len = abs(theta) * radius; // Yayın uzunluğunu, yayın açısı ve yarıçap çarpımı olarak hesapla.

  int i, segments = floor(len / THREAD_PER_STEP); // Yayı segmentlere böl, her segment bir adım olacak şekilde.

  float nx, ny, angle3, scale;

  // Segmentler boyunca yayı çiz
  for (i = 0; i < segments; ++i) {
    if (i == 0) 
      pen_up(); // İlk noktada kalem kaldır.
    else
      pen_down(); // Sonraki noktalarda kalem indir.

    scale = ((float)i) / ((float)segments); // İlerleme oranını hesapla.
    angle3 = (theta * scale) + angle1; // Geçerli segmentin açısını hesapla.
    nx = cx + cos(angle3) * radius; // X koordinatını hesapla.
    ny = cy + sin(angle3) * radius; // Y koordinatını hesapla.
    
    line_safe(nx, ny); // Güvenli çizgi fonksiyonu ile çiz.
  }

  line_safe(x, y); // Son noktaya çizgi çek.
  pen_up(); // Kalem kaldır.
}


//------------------------------------------------------------------------------
// Sanal çizim robotunun konumunu anında değiştirir, bu hareketin geçerliliğini kontrol etmez.
static void teleport(float x, float y) {
  posx = x;  // X pozisyonunu güncelle
  posy = y;  // Y pozisyonunu güncelle
  long l1, l2;
  IK(posx, posy, l1, l2);  // Yeni pozisyona göre L1 ve L2'yi hesapla
  laststep1 = l1;  // Son adımı güncelle
  laststep2 = l2;  // Son adımı güncelle
}



//==========================================================
// Eğik çizgi çizme programı
int workcnt=0; // Çalışma sayacı
void moveto(float x, float y) {
  long l1, l2;
  IK(x, y, l1, l2); // IK fonksiyonuyla x, y koordinatlarından L1 ve L2 uzunluklarını hesapla
  long d1 = l1 - laststep1; // Önceki adımdan bu adıma kadar olan L1 farkını hesapla
  long d2 = l2 - laststep2; // Önceki adımdan bu adıma kadar olan L2 farkını hesapla

  long ad1 = abs(d1); // d1'in mutlak değerini al
  long ad2 = abs(d2); // d2'nin mutlak değerini al
  int dir1 = d1 > 0 ? M1_REEL_IN : M1_REEL_OUT; // d1 pozitifse motoru içeri doğru, negatifse dışarı doğru hareket ettir
  int dir2 = d2 > 0 ? M2_REEL_IN : M2_REEL_OUT; // d2 pozitifse motoru içeri doğru, negatifse dışarı doğru hareket ettir
  long over = 0; // Fazla adım sayısını kontrol etmek için kullanılacak değişken
  long i; // Döngü değişkeni

  if (ad1 > ad2) { // Eğer L1 farkı L2 farkından büyükse
    for (i = 0; i < ad1; ++i) {
      m1.moveRelativeInSteps(dir1); // M1 motorunu belirlenen yönde adım adım hareket ettir
      over += ad2;
      if (over >= ad1) { // Eğer toplam adım sayısı L1 adım sayısını geçerse
        over -= ad1; // Fazlalığı çıkar
        m2.moveRelativeInSteps(dir2); // M2 motorunu bir adım hareket ettir
      }
      delayMicroseconds(step_delay); // Belirlenen mikrosaniye kadar bekle
    }
  } else { // Eğer L2 farkı L1 farkından büyükse veya eşitse
    for (i = 0; i < ad2; ++i) {
      m2.moveRelativeInSteps(dir2); // M2 motorunu belirlenen yönde adım adım hareket ettir
      over += ad1;
      if (over >= ad2) { // Eğer toplam adım sayısı L2 adım sayısını geçerse
        over -= ad2; // Fazlalığı çıkar
        m1.moveRelativeInSteps(dir1); // M1 motorunu bir adım hareket ettir
      }
      delayMicroseconds(step_delay); // Belirlenen mikrosaniye kadar bekle
    }
  }

  laststep1 = l1; // Son L1 uzunluğunu güncelle
  laststep2 = l2; // Son L2 uzunluğunu güncelle
  posx = x; // X pozisyonunu güncelle
  posy = y; // Y pozisyonunu güncelle
}


//------------------------------------------------------------------------------
// Uzun mesafelerde çizilen çizgiler genellikle eğri olabilir; bu yüzden, uzun çizgileri daha düz hale getirmek için parçalara ayırır.
static void line_safe(float x, float y) {
  float dx = x - posx;  // Mevcut konum ile hedef konum arasındaki x farkı
  float dy = y - posy;  // Mevcut konum ile hedef konum arasındaki y farkı

  float len = sqrt(dx * dx + dy * dy);  // İki nokta arasındaki mesafe
  
  if (len <= THREAD_PER_STEP) {  // Eğer uzunluk tek bir adıma eşit veya daha küçükse, doğrudan hareket et
    moveto(x, y);
    return;
  }
  
  // Çok uzun!
  long pieces = floor(len / THREAD_PER_STEP);  // Uzunluğu adım uzunluğuna bölerek parça sayısını hesapla
  float x0 = posx;
  float y0 = posy;
  float a;
  for (long j = 0; j <= pieces; ++j) {  // Her parça için
    a = (float)j / (float)pieces;  // İlerleme oranını hesapla

    moveto((x - x0) * a + x0, (y - y0) * a + y0);  // Her parçada ilgili noktaya hareket et
  }
  moveto(x, y);  // Son adımda hedef noktaya tam olarak hareket et
}



void line(float x, float y) {
  line_safe(x, y);  // Güvenli çizgi çizme işlevini kullanarak çizgi çiz
}


//------------------------------------------------------------------------------
// NC (Numerik Kontrol) kodlarını işler.
void nc(String st) {
  String xx, yy, zz;
  int ok = 1;
  st.toUpperCase();  // Gelen string'i büyük harfe çevir

  float x, y, z;
  int px = st.indexOf('X');  // X parametresinin başlangıç pozisyonu
  int py = st.indexOf('Y');  // Y parametresinin başlangıç pozisyonu
  int pz = st.indexOf('Z');  // Z parametresinin başlangıç pozisyonu
  if (px == -1 || py == -1) ok = 0;  // X veya Y bulunamazsa işlem yapma
  if (pz == -1) {
    pz = st.length();  // Z bulunamazsa string sonunu kullan
  } else {
    zz = st.substring(pz + 1);
    z = zz.toFloat();  // Z değerini float'a çevir
    if (z > 0) pen_up();  // Z pozitifse kalem kaldır
    if (z <= 0) pen_down();  // Z sıfır veya negatifse kalem indir
  }

  xx = st.substring(px + 1, py);  // X değerini çıkar
  yy = st.substring(py + 1, pz);  // Y değerini çıkar
  
  xx.trim();  // X değerindeki boşlukları temizle
  yy.trim();  // Y değerindeki boşlukları temizle

  if (ok) line(xx.toFloat(), yy.toFloat());  // Eğer her şey uygunsa çizgi çiz
}


void animateModDrawing() {
  const char* message = "Preparing";  // Gösterilecek mesaj
  int msgLength = strlen(message);  // Mesajın uzunluğu
  int displayWidth = u8x8.getCols();  // Ekran genişliği

  // Mesajı ekranın ortasına yerleştir
  int startPos = (displayWidth - msgLength) / 3;  // Başlangıç pozisyonunu hesapla

  for (int size = 1; size <= 2; size++) {  // Boyutu 1'den 2'ye çıkar
    u8x8.clear();  // Ekranı temizle
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);  // Büyük font kullan
    for (int j = 0; j < msgLength; j++) {
      // Mesajı karakter karakter çiz
      u8x8.draw1x2String(startPos + j, 1, message + j);  // Y konumu 1 olarak ayarla ki ortada olsun
      delay(100);  // Animasyon hızını ayarla
    }
    delay(100);  // Her boyutta biraz beklet
  }

  // Mesajı sola doğru kaydırarak sil
  for (int pos = startPos; pos >= -msgLength; pos--) {
    u8x8.clear();  // Ekranı temizle
    u8x8.draw1x2String(pos, 1, message);  // Mesajı güncel pozisyonda göster
    delay(100);  // Animasyon hızını ayarla
  }
}

// SD KART KONTROL 
void checkSDCard() {
    if (!SD.begin()) {
        Serial.println("SD Kart Takılı Değil!");
        // Ekran veya LED ile kullanıcıya bilgi verilebilir
    } else {
        Serial.println("SD Kart Hazır.");
    }
}


//------------------------------------------------------------------------------
// Çalışma durumunu OLED ekranda gösterir.

void working(void) {
  // Eğer ilk çalışma ise ya da belirli bir zaman geçtiyse animasyonu çalıştır
  u8x8.clear(); 
    animateModDrawing();  // Animasyonlu "hazırlanıyor" başlangıç ekrana yaz
      // modWorking mesajını ekrana yaz
  u8x8.clear();  // Mesajdan önce ekrani tekrar temizle
  u8x8.draw1x2String(1, 3, "mDrawing..."); // Ekranda mesajı göster
}


//------------------------------------------------------------------------------
// Belirtilen dosya isminden çizim yapar.
void drawfile(String filename) {
  String rd = "";
  int line = 0;
  char rr = 0;
  myFile = SD.open(filename);  // SD karttan dosyayı aç
  if (myFile) {
    working();  // Çalışma durumunu göster
    while (myFile.available()) {  // Dosya sonuna kadar oku
      rr = myFile.read();  // Bir karakter oku

      if (rr == char(10)) {  // Eğer satır sonu karakteriyse
        line++;
        nc(rd);  // Okunan satırı NC işlem fonksiyonuna gönder
        rd = "";
      } else {
        rd += rr;  // Okunan karakteri satır sonuna kadar ekle
      }
    }
    myFile.close();  // Dosyayı kapat
  } else {
    u8x8.clear();
    u8x8.drawString(3, 3, "putARTinSD");  // Dosya bulunamazsa hata mesajı göster
    delay(2000);  // 2 saniye bekle
  }
}


void setup() {
  // Kurulum için buradaki kodlar yalnızca bir kez çalıştırılır:
  Serial.begin(BAUD); // Seri iletişim başlatılıyor, baud rate olarak BAUD sabiti kullanılıyor.
  pinMode(BEEP, OUTPUT); // BEEP pinini çıkış olarak ayarla.
  digitalWrite(BEEP, LOW); // Başlangıçta BEEP pinini düşük seviyeye (kapalı) ayarla.
  u8x8.begin(); // U8x8 kütüphanesi ile ekran başlatılıyor.
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r); // Ekran için font ayarlanıyor.
  u8x8.draw1x2String(1, 3, "-- mDrawBOT --"); // Ekranın 1,3 konumuna "Hoş Geldiniz!" mesajını yaz.
 


  // Adım motorlarına bağlantı pinleri atanıyor:
  m1.connectToPins(7,8,9,10); // M1 motoru için UNO'nun 7, 8, 9 ve 10 numaralı pinleri kullanılıyor.
  m2.connectToPins(2,3,5,6); // M2 motoru için UNO'nun 2, 3, 5 ve 6 numaralı pinleri kullanılıyor.
  m1.setSpeedInStepsPerSecond(10000); // M1 motorunun saniyede atım hızını 10000 olarak ayarla.
  m1.setAccelerationInStepsPerSecondPerSecond(100000); // M1 motorunun saniyede atım ivmesini 100000 olarak ayarla.
  m2.setSpeedInStepsPerSecond(10000); // M2 motorunun saniyede atım hızını 10000 olarak ayarla.
  m2.setAccelerationInStepsPerSecondPerSecond(100000); // M2 motorunun saniyede atım ivmesini 100000 olarak ayarla.
  
  pen.attach(A0); // Kalem servo motorunu A0 pinine bağla.
  ps = PEN_UP_ANGLE; // Kalem açısını kaldırılmış pozisyona ayarla.
  pen.write(ps); // Servo motoruna kalem açısını uygula.

  teleport(0, 0); // Çizim başlangıç noktasını (0,0) olarak ayarla.
  mode_scale = 1; // Çizim ölçeklendirmesini 1 olarak ayarla (varsayılan, ölçeklendirme yok).
  
  delay(1000); // 1 saniye bekle
  u8x8.clear();  // Ekranı temizle
}

int KeyCheck(void) {
  int value = 0;  // Döndürülecek değer başlangıçta 0 olarak ayarlanır.
  int n = analogRead(ADKEY);  // ADKEY pininden analog değer okunur.

  if (n < 1020) {  // Okunan değer 1020'den küçükse tuş algılaması yap
    if (n < (KEYDOWN + 10)) {  // KEYDOWN değerine çok yakınsa
      if (n > (KEYDOWN - 10)) {  // KEYDOWN değerinden 10 birim az ya da fazla ise
        value = 1;  // Aşağı tuşuna basıldı
      }
    }  
    if (n < (KEYENTER + 10)) {  // KEYENTER değerine çok yakınsa
      if (n > (KEYENTER - 10)) {  // KEYENTER değerinden 10 birim az ya da fazla ise
        value = 2;  // Giriş (Enter) tuşuna basıldı
      }
    } 
    if (n < (KEYESC + 10)) {  // KEYESC değerine çok yakınsa
      if (n > (KEYESC - 10)) {  // KEYESC değerinden 10 birim az ya da fazla ise
        value = 3;  // İptal (ESC) tuşuna basıldı
      }
    } 
  }
  else {
    value = 0;  // Hiçbir tuşa basılmamışsa
  }
  
  return value;  // Tuşa basılma durumunu döndür
}


void beep(void) {
  int i = 5;  // Bip sesinin tekrar sayısı
  m1.disableMotor();  // M1 motorunu devre dışı bırak
  m2.disableMotor();  // M2 motorunu devre dışı bırak

  if (!BEEP_SW) return;  // Eğer BEEP_SW (bip sesi anahtarı) kapalıysa fonksiyondan çık

  while (i--) {  // 5 kez tekrarla
    digitalWrite(BEEP, HIGH);  // BEEP pinini yüksek seviyeye çek (bip sesi çıkar)
    delay(100);  // 300 milisaniye bekle
    digitalWrite(BEEP, LOW);  // BEEP pinini düşük seviyeye çek (bip sesini durdur)
    delay(200);  // 300 milisaniye daha bekle
  }
}

void simpleBeep() {
    int durations[] = {100, 200, 100, 200, 100, 200, 100, 200}; // Bip süreleri (milisaniye cinsinden)
    for (int i = 0; i < 8; i++) {
        digitalWrite(BEEP, HIGH); // Buzzer'ı aç
        delay(durations[i]); // Süre kadar bekle
        digitalWrite(BEEP, LOW); // Buzzer'ı kapat
        delay(durations[i]); // Süre kadar bekle
    }
}



void loop() {
  int keyvalue = 3;
  int maincase = 1;
  int subcase = 1;
  bool inModARTMenu = false;
  pen_up();

  while (true) {
    if (keyvalue != 0) {
      while (KeyCheck());

      if (keyvalue == 1) { // Select
        if (!inModARTMenu) {
          if (maincase < 10) {
            if (maincase < 3) maincase++;
            else maincase = 1;
          } else if (maincase >= 30 && maincase <= 33) {
            if (maincase == 33) maincase = 30;
            else maincase++;
          } else {
            maincase = 30;
          }
        } else {
          if (subcase < 3) subcase++;
          else subcase = 1;
        }
      } else if (keyvalue == 2) { // Enter
        if (!inModARTMenu) {
          if (maincase == 1) {
            inModARTMenu = true;
            subcase = 1;
          } else if (maincase == 2) {
            // Start SD card process
            if (!SD.begin(4)) {
              // SD card initialization failed!
              u8x8.clear();
              u8x8.draw1x2String(0, 0, "Put SD Card!");
              delay(2000);
              u8x8.clear();
            } else {
              pen_up();
              drawfile("main.nc");
              simpleBeep();
              u8x8.clear();
            }
          } else if (maincase == 30) {
            moveto(0, 251);
          } else if (maincase == 31) {
            moveto(0, 0);
          } else if (maincase == 32) {
            moveto(-251, 0); // Move to the left
          } else if (maincase == 33) {
            moveto(251, 0); // Move to the right
          } else {
            maincase *= 10;
            u8x8.clear();
          }
        } else {
          if (subcase == 1) {
            // Demo 1: Draw a circle
            circle(0, 0, 50, 50);
          } else if (subcase == 2) {
            // Demo 2: Execute modART
            modART();
          } else if (subcase == 3) {
            // Demo 3: Draw a heart curve
            heart_curve(0, 0, 2, 2);
          }
          beep();
          u8x8.clear();
          inModARTMenu = false;
        }
      } else if (keyvalue == 3) { // ESC
        if (!inModARTMenu) {
          if (maincase > 9) {
            maincase /= 10;
          }
          u8x8.clear();
        } else {
          inModARTMenu = false;
          u8x8.clear();
        }
      }

      if (!inModARTMenu) {
        switch (maincase) {
          case 1: {
            u8x8.clear();
            u8x8.draw1x2String(0, 1, "- modART Demo");
            u8x8.draw1x2String(0, 3, "  SD CARD");
            u8x8.draw1x2String(0, 5, "  SET WALL");
          } break;
          case 2: {
            u8x8.clear();
            u8x8.draw1x2String(0, 1, "  modART Demo");
            u8x8.draw1x2String(0, 3, "- SD CARD");
            u8x8.draw1x2String(0, 5, "  SET WALL");
          } break;
          case 3: {
            u8x8.clear();
            u8x8.draw1x2String(0, 1, "  modART Demo");
            u8x8.draw1x2String(0, 3, "  SD CARD");
            u8x8.draw1x2String(0, 5, "- SET WALL");
          } break;
          case 30: {
            u8x8.clear();
            u8x8.draw1x2String(0, 0, "- UP");
            u8x8.draw1x2String(0, 2, "  DOWN");
            u8x8.draw1x2String(0, 4, "  LEFT");
            u8x8.draw1x2String(0, 6, "  RIGHT");
          } break;
          case 31: {
            u8x8.clear();
            u8x8.draw1x2String(0, 0, "  UP");
            u8x8.draw1x2String(0, 2, "- DOWN");
            u8x8.draw1x2String(0, 4, "  LEFT");
            u8x8.draw1x2String(0, 6, "  RIGHT");
          } break;
          case 32: {
            u8x8.clear();
            u8x8.draw1x2String(0, 0, "  UP");
            u8x8.draw1x2String(0, 2, "  DOWN");
            u8x8.draw1x2String(0, 4, "- LEFT");
            u8x8.draw1x2String(0, 6, "  RIGHT");
          } break;
          case 33: {
            u8x8.clear();
            u8x8.draw1x2String(0, 0, "  UP");
            u8x8.draw1x2String(0, 2, "  DOWN");
            u8x8.draw1x2String(0, 4, "  LEFT");
            u8x8.draw1x2String(0, 6, "- RIGHT");
          } break;
          default: break;
        }
      } else {
        switch (subcase) {
          case 1: {
            u8x8.clear();
            u8x8.draw1x2String(0, 1, "- Demo 1");
            u8x8.draw1x2String(0, 3, "  Demo 2");
            u8x8.draw1x2String(0, 5, "  Demo 3");
          } break;
          case 2: {
            u8x8.clear();
            u8x8.draw1x2String(0, 1, "  Demo 1");
            u8x8.draw1x2String(0, 3, "- Demo 2");
            u8x8.draw1x2String(0, 5, "  Demo 3");
          } break;
          case 3: {
            u8x8.clear();
            u8x8.draw1x2String(0, 1, "  Demo 1");
            u8x8.draw1x2String(0, 3, "  Demo 2");
            u8x8.draw1x2String(0, 5, "- Demo 3");
          } break;
          default: break;
        }
      }
    }
    keyvalue = KeyCheck();
  }
}

void heart_curve(int xx, int yy, float x_scale, float y_scale) {
    float xa, ya;
    pen_up();
    moveto(xx, yy + y_scale * 7);  // Başlangıç noktasına git
    pen_down();

    // Kalp şeklini çizmek için döngü
    for (float i = 0; i <= 6.28; i += 3.14 / 180) {
        xa = x_scale * pow(sin(i), 3) * 15;
        ya = y_scale * (15 * cos(i) - 5 * cos(2 * i) - 2 * cos(3 * i) - cos(4 * i));
        line_safe(xa + xx, ya + yy);  // Kalp şeklini çiz
    }
    pen_up();
}

void rectangle(float xx, float yy, float dx, float dy, float angle) {
    float six, csx, siy, csy;
    dx /= 2;
    dy /= 2;

    six = sin(angle / 180 * 3.14) * dx;
    csx = cos(angle / 180 * 3.14) * dx;
    siy = sin(angle / 180 * 3.14) * dy;
    csy = cos(angle / 180 * 3.14) * dy;

    pen_up();
    line_safe(csx - siy + xx, six + csy + yy);
    pen_down();
    line_safe(xx - csx - siy, csy - six + yy);
    line_safe(xx - csx + siy, yy - csy - six);
    line_safe(csx + siy + xx, six - csy + yy);
    line_safe(csx - siy + xx, six + csy + yy);
    pen_up();
}

void box(float xx, float yy, float dx, float dy) {
    pen_up();
    line_safe(xx, yy);
    pen_down();
    delay(TPD);
    line_safe(xx + dx, yy);
    delay(TPD);
    line_safe(xx + dx, yy + dy);
    delay(TPD);
    line_safe(xx, yy + dy);
    delay(TPD);
    line_safe(xx, yy);
    pen_up();
}

void circle(float xx, float yy, float radius_x, float radius_y) {
    float rx, ry;
    float st = 3.14159 / 90; // Daireyi parçalara ayırma hassasiyeti
    pen_up();
    line(xx + radius_x, yy);
    pen_down();
    for (float i = 0; i < 6.28318; i += st) {
        rx = cos(i) * radius_x;
        ry = sin(i) * radius_y;
        line(xx + rx, yy + ry);
    }
    pen_up();
}

void modART() {
    pen_up();
    box(-45, 0, 90, 90);
    moveto(-15, 0);
    pen_down();
    line(-15, 90);
    pen_up();
    moveto(15, 90);
    pen_down();
    line(15, 0);
    pen_up();
    moveto(-45, 30);
    pen_down();
    line(45, 30);
    pen_up();
    moveto(45, 60);
    pen_down();
    line(-45, 60);
    pen_up();

    box(-42.5, 62.5, 25, 25);
    circle(0, 75, 12.5, 12.5);
    rectangle(30, 75, 17.7, 17.7, 45);

    rectangle(-30, 45, 17.7, 17.7, 45);
    box(-12.5, 32.5, 25, 25);
    circle(30, 45, 12.5, 12.5);

    circle(-30, 15, 12.5, 12.5);
    rectangle(0, 15, 17.7, 17.7, 45);
    box(17.5, 2.5, 25, 25);

    heart_curve(-45, -45, 2, 2);
}
