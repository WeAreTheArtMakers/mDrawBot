// mDrawBot Duvar Çizim Robotu Kodları
// WATAM - @STUDIOBRN WeAreTheArtMakers.com 
// OPEN-SOURCE modARTCollective 

// Gerekli Kütüphaneleri yüklüyoruz...
#include <TinyStepper_28BYJ_48.h>		
#include <Servo.h>
#include <SD.h>       
#include <U8x8lib.h>  
#include <avr/pgmspace.h>

//U8X8 SSD1306 128x64 boyutunda bir OLED ekranı başlatır. Bu ekran I2C protokolü üzerinden iletişim kurar.
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// PROGMEM ile metin mesajlarını tanımlama
const char msg_modart_demo[] PROGMEM = "modART Demo";
const char msg_sd_card[] PROGMEM = "SD CARD";
const char msg_set_wall[] PROGMEM = "SET WALL";
const char msg_err[] PROGMEM = "Err";
const char msg_your_gcode[] PROGMEM = "Your Gcode";
const char msg_rename_mod[] PROGMEM = "Rename mod.nc";
const char msg_put_in_sd[] PROGMEM = "Put in SD CARD!";
const char msg_up[] PROGMEM = "UP";
const char msg_down[] PROGMEM = "DOWN";
const char msg_left[] PROGMEM = "LEFT";
const char msg_right[] PROGMEM = "RIGHT";
const char msg_modcircle[] PROGMEM = "modCircle";
const char msg_moddemoart[] PROGMEM = "modDemoART";
const char msg_modheart[] PROGMEM = "modHeart";

#define STEPS_PER_TURN  (2048)  // Adım motoru 360 dereceyi 2048 adımda döner
#define SPOOL_DIAMETER  (35)    // Makaranın çapı (mm)
#define SPOOL_CIRC      (SPOOL_DIAMETER * 3.1416)  // Makaranın çevresi: 35 * 3.14 = 109.956 (mm)
#define TPS             (SPOOL_CIRC / STEPS_PER_TURN)  // Adım motorunun adım mesafesi, minimum çözünürlük,
// ipi her adımda çektiği mesafe: 0.053689 mm

#define step_delay      1   // Adım motorunun her adımındaki bekleme süresi (mikrosaniye)
#define TPD             300   // Dönüş sırasında bekleme süresi (milisaniye)

// İki motorun dönüş yönü: 1 İleri, -1 Geri
// Görüntüyü dikey olarak ters çevirmek için in ve out yönlerini ayarlayın
#define M1_REEL_OUT     -1     // İpi sal
#define M1_REEL_IN      1      // İpi çek
#define M2_REEL_OUT     1      // İpi sal
#define M2_REEL_IN      -1     // İpi çek

static long laststep1, laststep2; // İpin mevcut uzunluğu, Kalemin pozisyonu

#define X_SEPARATION  350           // İki ipin üstündeki yatay mesafe (mm)
#define LIMXMAX       ( X_SEPARATION * 0.5)   // x ekseni maksimum değeri, 0 çizim tahtasının merkezindedir
#define LIMXMIN       (-X_SEPARATION * 0.5)   // x ekseni minimum değeri

// Dikey mesafe parametresi: Pozitif değer, çizim tahtasının altında yer alır, teorik olarak
// çizim tahtası yeterince büyük olduğu sürece sonsuz büyüklükte olabilir, negatif değer
// alan kalemin üstündedir (güç verilmeden önce)
#define LIMYMAX         (-250)   // y ekseni maksimum değeri, çizim tahtasının altındaki değer
#define LIMYMIN         (250)    // y ekseni minimum değeri, çizim tahtasının üstündeki değer
// Sol ve sağ iplerin sabit noktasının kalemle olan dikey mesafesi
// olabildiğince doğru yerleştirilmelidir. Hata çok büyükse, çarpıklık olur.
// Değer azaltılırsa, çizim ince ve uzun olur, değer artırılırsa, çizim geniş ve kısa olur

// Kalem kaldırma servosunun açı parametresinin spesifik değeri,
// salınım kolunun pozisyonuna bağlıdır ve ayarlanması gerekir
#define PEN_UP_ANGLE    85  // Kalemi kaldır
#define PEN_DOWN_ANGLE  70  // Kalemi indir
// =============================================

#define PEN_DOWN 1  // Kalem durumu: İndirilmiş
#define PEN_UP 0    // Kalem durumu: Kaldırılmış

struct point { 
  float x; 
  float y; 
  float z; 
};

struct point actuatorPos; // Aktüatör pozisyonu

// Plotter pozisyonu
static float posx;
static float posy;
static float posz;  // Kalem durumu
static float feed_rate = 0;

// Kalem durumu (kaldırma, indirme)
static int ps;

/* G kodu iletişim parametreleri */
#define BAUD            (115200)    // 9600, 57600, 115200 
#define MAX_BUF         (64)      

// Seri iletişim alımı
static int sofar;               // Seri buffer ilerleme durumu

static float mode_scale;   // Oran

File myFile;

Servo pen; // Kalem
#define BEEP_SW 1 // İş sonrası buzzer uyarısı
#define BEEP A2 // Buzzer pini
#define ADKEY A1 // Analog anahtar pini
#define KEYDOWN 330 // Aşağı tuşu değeri
#define KEYENTER 90 // Giriş tuşu değeri
#define KEYESC 517 // Çıkış tuşu değeri

TinyStepper_28BYJ_48 m1; //(7,8,9,10);  // M1 L Motoru in1~4 Pinleri UNO 7 8 9 10
TinyStepper_28BYJ_48 m2; //(2,3,5,6);  // M2 R Motoru in1~4 Pinleri UNO 2 3 5 6

// Fonksiyon prototipleri
void displayMessage(int x, int y, const char* msg);
void handleSDCard();

//------------------------------------------------------------------------------

void FK(float l1, float l2, float &x, float &y) {
  float a = l1 * TPS;
  float b = X_SEPARATION;
  float c = l2 * TPS;
  
  // Yöntem 1
  float theta = acos((a * a + b * b - c * c) / (2.0 * a * b));
  x = cos(theta) * l1 + LIMXMIN;
  y = sin(theta) * l1 + LIMYMIN;          

  // Yöntem 2
  /*   
  float theta = (a * a + b * b - c * c) / (2.0 * a * b);
  x = theta * l1 + LIMXMIN;
  y = sqrt(1.0 - theta * theta) * l1 + LIMYMIN;
  */
}


//------------------------------------------------------------------------------

void IK(float x,float y,long &l1, long &l2)
{
  float dy = y - LIMYMIN;
  float dx = x - LIMXMIN;
  l1 = round(sqrt(dx*dx+dy*dy) / TPS);
  dx = x - LIMXMAX;
  l2 = round(sqrt(dx*dx+dy*dy) / TPS);
}

//------------------------------------------------------------------------------

void pen_state(int pen_st)
{
  if(pen_st==PEN_DOWN)
  {
    ps=PEN_DOWN_ANGLE;
    // Serial.println("Pen down");
  } 
  else 
  {
    ps=PEN_UP_ANGLE;
    //Serial.println("Pen up");
  }
  pen.write(ps);
}

void pen_down()
{ 
  if (ps==PEN_UP_ANGLE)
  {
    ps=PEN_DOWN_ANGLE;
    pen.write(ps);
    delay(TPD);   
  }
}

void pen_up()
{
  if (ps==PEN_DOWN_ANGLE)
  {
    ps=PEN_UP_ANGLE;
    pen.write(ps);
  }  
}

//------------------------------------------------------------------------------
// returns angle of dy/dx as a value from 0...2PI
static float atan3(float dy, float dx) {
  float a = atan2(dy, dx);
  if (a < 0) a = (PI * 2.0) + a;
  return a;
}

//------------------------------------------------------------------------------

static void arc(float cx, float cy, float x, float y,  float dir) {
  // get radius
  float dx = posx - cx;
  float dy = posy - cy;
  float radius = sqrt(dx * dx + dy * dy);

  // find angle of arc (sweep)
  float angle1 = atan3(dy, dx);
  float angle2 = atan3(y - cy, x - cx);
  float theta = angle2 - angle1;

  if (dir > 0 && theta < 0) angle2 += 2 * PI;
  else if (dir < 0 && theta>0) angle1 += 2 * PI;

  // get length of arc
  // float circ=PI*2.0*radius;
  // float len=theta*circ/(PI*2.0);
  // simplifies to
  float len = abs(theta) * radius;

  int i, segments = floor(len / TPS);

  float nx, ny, nz, angle3, scale;

  for (i = 0; i < segments; ++i) {

    if (i==0) 
      pen_up();
    else
      pen_down();  
    scale = ((float)i) / ((float)segments);

    angle3 = (theta * scale) + angle1;
    nx = cx + cos(angle3) * radius;
    ny = cy + sin(angle3) * radius;
    // send it to the planner
    line_safe(nx, ny);
  }

  line_safe(x, y);
  pen_up();
}

//------------------------------------------------------------------------------
// instantly move the virtual plotter position
// does not validate if the move is valid
static void teleport(float x, float y) {
  posx = x;
  posy = y;
  long l1,l2;
  IK(posx, posy, l1, l2);
  laststep1 = l1;
  laststep2 = l2;
}

//==========================================================

int workcnt=0;
void moveto(float x,float y) 
{
  long l1,l2;
  IK(x,y,l1,l2);
  long d1 = l1 - laststep1;
  long d2 = l2 - laststep2;

  long ad1=abs(d1);
  long ad2=abs(d2);
  int dir1=d1>0 ? M1_REEL_IN : M1_REEL_OUT;
  int dir2=d2>0 ? M2_REEL_IN : M2_REEL_OUT;
  long over=0;
  long i;

  if(ad1>ad2) {
    for(i=0;i<ad1;++i) {
      m1.moveRelativeInSteps(dir1);
      over+=ad2;
      if(over>=ad1) {
        over-=ad1;
        m2.moveRelativeInSteps(dir2);
      }
      delayMicroseconds(step_delay);
     }
  } 
  else {
    for(i=0;i<ad2;++i) {
      m2.moveRelativeInSteps(dir2);
      over+=ad1;
      if(over>=ad2) {
        over-=ad2;
        m1.moveRelativeInSteps(dir1);
      }
      delayMicroseconds(step_delay);
    }
  }

  laststep1=l1;
  laststep2=l2;
  posx=x;
  posy=y;
}

//------------------------------------------------------------------------------
static void line_safe(float x,float y) {
  // split up long lines to make them straighter?
  float dx=x-posx;
  float dy=y-posy;

  float len=sqrt(dx*dx+dy*dy);
  
  if(len<=TPS) {
    moveto(x,y);
    return;
  }
  
  // too long!
  long pieces=floor(len/TPS);
  float x0=posx;
  float y0=posy;
  float a;
  for(long j=0;j<=pieces;++j) {
    a=(float)j/(float)pieces;

    moveto((x-x0)*a+x0,
         (y-y0)*a+y0);
  }
  moveto(x,y);
}

void line(float x,float y) 
{
  line_safe(x,y);
}

//********************************
void nc(String st)
{
  String xx,yy,zz;
  int ok=1;
  st.toUpperCase();
  
  float x,y,z;
  int px,py,pz;
  px = st.indexOf('X');
  py = st.indexOf('Y');
  pz = st.indexOf('Z');
  if (px==-1 || py==-1) ok=0; 
  if (pz==-1) 
  {
    pz=st.length();
  }
  else
  {   
    zz = st.substring(pz+1,st.length());
    z  = zz.toFloat();
    if (z>0)  pen_up();
    if (z<=0) pen_down();
  }

  xx = st.substring(px+1,py);
  yy = st.substring(py+1,pz);
  
  xx.trim();
  yy.trim();

  if (ok) line(xx.toFloat(),yy.toFloat()); 
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


//**********************
void working(void) {
  // Eğer ilk çalışma ise ya da belirli bir zaman geçtiyse animasyonu çalıştır
  u8x8.clear(); 
    animateModDrawing();  // Animasyonlu "hazırlanıyor" başlangıç ekrana yaz
      // modWorking mesajını ekrana yaz
  u8x8.clear();  // Mesajdan önce ekrani tekrar temizle
  u8x8.draw1x2String(1, 3, "mDrawing..."); // Ekranda mesajı göster
}
//**********************
void drawfile( String filename)
{
  String rd="";
  int line=0;
  char rr=0;
  //Serial.print(F("["));
  //Serial.print(filename);
  myFile = SD.open(filename);
  if (myFile) 
  {
    //Serial.println(F("] Opened"));
    working();
    while (myFile.available()) {
    rr=myFile.read();
      
    if (rr == char(10)) 
    {
      line++;
    //          Serial.print(F("Run nc #"));
    //          Serial.print(line);
    //          Serial.println(" : "+rd);         
      nc(rd);
      rd="";
    }
    else
    {
       rd+=rr;      
    }  
    }
    myFile.close(); 
  }
  else
  {
    u8x8.clear();
    u8x8.drawString(0, 3, "Nofile");
    delay(2000);
  }
}

void setup() {
  // Kurulumu buraya yapıyoruz başlangıçta ne çalışacaksa onu ekleyelim.
  Serial.begin(BAUD);
  pinMode(BEEP,OUTPUT);
  digitalWrite(BEEP,LOW);
  u8x8.begin();
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
  u8x8.draw1x2String(2, 3, "--W A T A M--");

  m1.connectToPins(7,8,9,10); //M1 L Motor
  m2.connectToPins(2,3,5,6);  //M2 R Motor
  m1.setSpeedInStepsPerSecond(10000);
  m1.setAccelerationInStepsPerSecondPerSecond(100000);
  m2.setSpeedInStepsPerSecond(10000);
  m2.setAccelerationInStepsPerSecondPerSecond(100000);
   
  // Kalem kaldırma servo motoru
  pen.attach(A0);
  ps = PEN_UP_ANGLE;
  pen.write(ps);

  // Kalemin mevcut konumunu 0, 0 olarak ayarla
  teleport(0, 0);
  // Ölçekleme oranı
  mode_scale = 1;
  //Serial.println(F("Test OK!"));
  delay(1500);
  u8x8.clear();

}

int KeyCheck(void)
{
  int value=0;
  int n = analogRead(ADKEY);
  if(n<1020)
  {
    if(n<(KEYDOWN+10))
    {
      if(n>(KEYDOWN-10))
      {
        value=1;
      }  
    }  
    if(n<(KEYENTER+10))
    {
      if(n>(KEYENTER-10))
      {
        value=2;
      }  
    } 
    if(n<(KEYESC+10))
    {
      if(n>(KEYESC-10))
      {
        value=3;
      }  
    } 
  }
  else
  {
    value=0;  
  }
  return value;
}

void beep(void)
{
  int i=6;
  m1.disableMotor();
  m2.disableMotor();
  if (!BEEP_SW) return;
  while(i--)
  {
    digitalWrite(BEEP,HIGH);   
    delay(30);    
    digitalWrite(BEEP,LOW);
    delay(60);
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

      if (keyvalue == 1) { // Seçim
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
      } else if (keyvalue == 2) { // ENTER gibi
        if (!inModARTMenu) {
          if (maincase == 1) {
            inModARTMenu = true;
            subcase = 1;
          } else if (maincase == 2) {
            handleSDCard();
          } else if (maincase == 30) {
            moveto(0, 251);
            beep();
          } else if (maincase == 31) {
            moveto(0, 0);
            beep();
          } else if (maincase == 32) {
            moveto(-251, 0); // Sola git
            beep();
          } else if (maincase == 33) {
            moveto(251, 0); // Sağa Git 
            beep();
          } else {
            maincase *= 10;
            u8x8.clear();
          }
        } else {
          if (subcase == 1) {
            // Demo 1: Daire Çiz
            circle(0, 0, 50, 50);
            beep();
          } else if (subcase == 2) {
            // Demo 2:  modART
            modART();
            beep();
          } else if (subcase == 3) {
            // Demo 3: Kalp Çiz
            heart_curve(0, 0, 2, 2);
            beep();
          }
          beep();
          u8x8.clear();
          inModARTMenu = false;
        }
      } else if (keyvalue == 3) { // Geri Tuşu
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

  
      u8x8.clear();

      if (!inModARTMenu) {
        displayMenu(maincase, true);
      } else {
        displayMenu(subcase, false);
      }
    }
    keyvalue = KeyCheck();
  }
}

void displayMenu(int caseValue, bool isMain) {
  if (isMain) {
    switch (caseValue) {
      case 1: {
        displayMessage(0, 1, msg_modart_demo, true);
        displayMessage(0, 3, msg_sd_card, false);
        displayMessage(0, 5, msg_set_wall, false);
      } break;
      case 2: {
        displayMessage(0, 1, msg_modart_demo, false);
        displayMessage(0, 3, msg_sd_card, true);
        displayMessage(0, 5, msg_set_wall, false);
      } break;
      case 3: {
        displayMessage(0, 1, msg_modart_demo, false);
        displayMessage(0, 3, msg_sd_card, false);
        displayMessage(0, 5, msg_set_wall, true);
      } break;
      case 30: {
        displayMessage(0, 0, msg_up, true);
        displayMessage(0, 2, msg_down, false);
        displayMessage(0, 4, msg_left, false);
        displayMessage(0, 6, msg_right, false);
      } break;
      case 31: {
        displayMessage(0, 0, msg_up, false);
        displayMessage(0, 2, msg_down, true);
        displayMessage(0, 4, msg_left, false);
        displayMessage(0, 6, msg_right, false);
      } break;
      case 32: {
        displayMessage(0, 0, msg_up, false);
        displayMessage(0, 2, msg_down, false);
        displayMessage(0, 4, msg_left, true);
        displayMessage(0, 6, msg_right, false);
      } break;
      case 33: {
        displayMessage(0, 0, msg_up, false);
        displayMessage(0, 2, msg_down, false);
        displayMessage(0, 4, msg_left, false);
        displayMessage(0, 6, msg_right, true);
      } break;
      default: break;
    }
  } else {
    switch (caseValue) {
      case 1: {
        displayMessage(0, 1, msg_modcircle, true);
        displayMessage(0, 3, msg_moddemoart, false);
        displayMessage(0, 5, msg_modheart, false);
      } break;
      case 2: {
        displayMessage(0, 1, msg_modcircle, false);
        displayMessage(0, 3, msg_moddemoart, true);
        displayMessage(0, 5, msg_modheart, false);
      } break;
      case 3: {
        displayMessage(0, 1, msg_modcircle, false);
        displayMessage(0, 3, msg_moddemoart, false);
        displayMessage(0, 5, msg_modheart, true);
      } break;
      default: break;
    }
  }
}

void displayMessage(int x, int y, const char* msg, bool isSelected) {
  char buffer[20];
  strcpy_P(buffer, (PGM_P)msg);
  if (isSelected) {
    char tempBuffer[20];
    snprintf(tempBuffer, sizeof(tempBuffer), "-%s", buffer);
    u8x8.draw1x2String(x, y, tempBuffer);
  } else {
    u8x8.draw1x2String(x, y, buffer);
  }
}

void handleSDCard() {
  if (!SD.begin(4)) {
    u8x8.clear();
    displayMessage(0, 0, msg_err, false);
    delay(2000);
    u8x8.clear();
  } else {
    pen_up();
    drawfile("mod.nc");
    beep();
    u8x8.clear();
  }
}


void butterfly_curve(int xx,int yy,int lines,int x_scale,int y_scale) {
  float xa,ya,p,e;
  pen_up();
  moveto(xx,yy + y_scale  * 0.71828);
  pen_down();
  
  for(float i=0;i<6.28*lines;i+=3.14/90) {       
    p=pow(sin(i/12),5);
    e=pow(2.71828,cos(i));

    xa=x_scale * sin(i) * (e - 2*cos(4*i) + p);
    ya=y_scale * cos(i) * (e - 2*cos(4*i) + p);
    line_safe(xa+xx,ya+yy); 
  }
  pen_up();
}  

void heart_curve(int xx,int yy,float x_scale,float y_scale) {
  float xa,ya;
  
  pen_up();
  moveto(xx,yy+y_scale * 7);
  pen_down();
  for(float i=0;i<=6.28;i+=3.14/180) {       
    xa=x_scale * pow(sin(i),3) * 15;
    ya=y_scale * (15*cos(i) -5*cos(2*i) - 2*cos(3*i) - cos(4*i));
    line_safe(xa+xx,ya+yy);  
  }
  pen_up();
} 

void rectangle(float xx,float yy,float dx,float dy,float angle) {
  float six,csx,siy,csy;
  dx/=2;
  dy/=2;

  six = sin(angle/180*3.14) * dx;
  csx = cos(angle/180*3.14) * dx;
  
  siy = sin(angle/180*3.14) * dy;
  csy = cos(angle/180*3.14) * dy;
  
  pen_up();
  line_safe(csx - siy + xx,six + csy + yy);
  pen_down();
  line_safe(xx - csx - siy,csy - six + yy);
  line_safe(xx - csx + siy,yy - csy - six);
  line_safe(csx + siy + xx,six - csy + yy);
  line_safe(csx - siy + xx,six + csy + yy);
  pen_up();
}

void box(float xx,float yy,float dx,float dy) {
  pen_up();
  line_safe(xx , yy);
  pen_down();
  delay(TPD);
  line_safe(xx + dx, yy);
  delay(TPD);
  line_safe(xx + dx, yy+ dy);
  delay(TPD);
  line_safe(xx , yy + dy);
  delay(TPD);
  line_safe(xx , yy);
  pen_up();
}

void circle(float xx,float yy,float radius_x,float radius_y) {
  float rx,ry;
  float st= 3.14159 / 90; //Circle division accuracy
  pen_up();
  line(xx+radius_x,yy);
  pen_down();
  for(float i=0;i<6.28318;i+=st) {
    rx = cos(i) * radius_x;
    ry = sin(i) * radius_y;
    line(xx+rx,yy+ry);
  }
  pen_up();
}

void modART() {
  heart_curve(-45,-45,2,2);
  butterfly_curve(45,-55,3,12,12);
}
