// LiquidCrystal kütüphanesini ekleyerek başlıyoruz.
#include <LiquidCrystal.h>

// LCD'ye bağlanacak olan pinlerin tanımlanması.
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

// LiquidCrystal sınıfından bir nesne oluşturuluyor.
// Bu nesne, LCD'nin kontrolü için kullanılacak.
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Arduino'nun başlangıçta çalıştıracağı setup fonksiyonu.
void setup()
{
  // LCD'nin başlatılması, 16 sütun ve 2 satırdan oluştuğunu belirtiyoruz.
  lcd.begin(16, 2);
  // LCD ekranında "mDrawBot" mesajının gösterilmesi.
  lcd.print("mDrawBot");
}

// Arduino'nun sürekli olarak çalıştıracağı loop fonksiyonu.
void loop()
{
  // Bu örnekte loop fonksiyonu içi boş.
  // Buraya ekleyeceğiniz kodlar her döngüde tekrar tekrar çalıştırılacaktır.
}
