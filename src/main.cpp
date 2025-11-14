// main.cpp - v9.1 STREAM PARSING EKLENDI
#include "MoveSalinim.h" 
#include <Arduino.h>
#include "Config.h"
#include "PulseAt.h"
#include "MoveTo.h"
#include "stepmotorenkoderiokuma.h"
#include "A0Filtre.h"
#include "KayitModulu.h"
#include "OynatmaModulu.h"
#include "CiftKayitModulu.h"
#include "CiftOynatmaModulu.h"

// ═══════════════════════════════════════════════════════════════
// ENCODER NESNELERİ
// ═══════════════════════════════════════════════════════════════
StepMotorEncoder zEnc(ENC2_A_PIN, ENC2_B_PIN);
StepMotorEncoder xEnc(ENC1_A_PIN, ENC1_B_PIN);
StepMotorEncoder bigEnc(ENC3_A_PIN, ENC3_B_PIN);

// ═══════════════════════════════════════════════════════════════
// DİNAMİK PARAMETRELER
// ═══════════════════════════════════════════════════════════════
static long bigFreqMin = 10;
static long bigFreqMax = 100;
static long zEncMin = 0;
static long zEncMax = 20000;
static long bigFreqRef = 30;
static float depoCapMm = 520.0;  // ✅ YENİ: Dinamik depo çapı (mm)

// ═══════════════════════════════════════════════════════════════
// ÇİFT KAYIT/OYNATMA X POZİSYONLARI
// ═══════════════════════════════════════════════════════════════
static long x1Pos = 0;
static long x2Pos = -10000;

// ═══════════════════════════════════════════════════════════════
// KOMUT BUFFER
// ═══════════════════════════════════════════════════════════════
static char cmdBuffer[64];
static uint16_t cmdIndex = 0;

// ═══════════════════════════════════════════════════════════════
// FONKSİYON PROTOTIPLERI
// ═══════════════════════════════════════════════════════════════
void yazdirMenu();
void handleCommand(const char* cmd);
void handleEncoderOku();
void handleA0Oku();
void handleCiftKayit();
void handleCiftOynatma();
void handleReset(char motor);
void handleX1Ayarla(const char* cmd);
void handleX2Ayarla(const char* cmd);
void handleXShow();
void handleBigRefAyarla(const char* cmd);
void handleBigRefShow();
void handleDepoCapAyarla(const char* cmd);
void handleDepoCapShow();

// ═══════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║      DEPO KAYNAĞI SİSTEMİ BAŞLATILIYOR        ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));
  
  Serial.println(F("[1/5] Encoder'lar başlatılıyor..."));
  zEnc.begin();
  xEnc.begin();
  bigEnc.begin();
  Serial.println(F("✓ Z, X, BIG encoder'lar hazır!\n"));
  
  Serial.println(F("[2/5] MoveTo modülü ayarlanıyor..."));
  moveToSetup(&zEnc, &xEnc, &bigEnc);
  Serial.println(F("✓ MoveTo hazır!\n"));
  
  Serial.println(F("[3/5] Kayıt modülü ayarlanıyor..."));
  kayitEncoderSetup(&bigEnc);
  Serial.println(F("✓ Kayıt modülü hazır!\n"));
  
  Serial.println(F("[4/5] Oynatma modülü ayarlanıyor..."));
  oynatmaEncoderSetup(&bigEnc, &zEnc);
  oynatmaParametreSetup(&bigFreqMin, &bigFreqMax, &zEncMin, &zEncMax);
  oynatmaRefHizSetup(&bigFreqRef);
  oynatmaDepoCapSetup(&depoCapMm);  // ✅ YENİ
  Serial.println(F("✓ Oynatma modülü hazır!\n"));
  
  Serial.println(F("[5/5] Çift Kayıt/Oynatma modülleri ayarlanıyor..."));
  ckEncoderSetup(&bigEnc, &xEnc);
  coEncoderSetup(&bigEnc, &xEnc, &zEnc);
  coParametreSetup(&bigFreqMin, &bigFreqMax, &zEncMin, &zEncMax);
  Serial.println(F("✓ Çift modüller hazır!\n"));

  msEncoderSetup(&xEnc);
  
  Serial.println(F("╔════════════════════════════════════════════════╗"));
  Serial.println(F("║            SİSTEM HAZIR! 🚀                    ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));
  
  pinMode(KAYNAK_ROLE_PIN, OUTPUT);
  digitalWrite(KAYNAK_ROLE_PIN, HIGH);
  
  yazdirMenu();
}

// ═══════════════════════════════════════════════════════════════
// LOOP
// ═══════════════════════════════════════════════════════════════
void loop() {
  moveToRun();
  kayitRun();
  oynatmaRun();
  ckRun();
  coRun();

  if (oynatmaAktifMi() && !ckAktifMi()) {
    msRun();
  }
  
  if (oynatmaTamamlandiMi() && msAktifMi()) {
    msDurdur();
    Serial.println(F("Oynatma bitti, salinim durduruldu!"));
  }
  
  // ═══════════════════════════════════════════════════════════════
  // SERIAL OKUMA
  // ═══════════════════════════════════════════════════════════════
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0';
        handleCommand(cmdBuffer);
        cmdIndex = 0;
      }
    }
    else if (cmdIndex < sizeof(cmdBuffer) - 1) {
      cmdBuffer[cmdIndex++] = c;
    }
  }
}

// ═══════════════════════════════════════════════════════════════
// KOMUT İŞLEYİCİSİ
// ═══════════════════════════════════════════════════════════════
void handleCommand(const char* cmd) {
  
  // ─────────────────────────────────────────────────────────────
  // [RST] RESET KOMUTU
  // ─────────────────────────────────────────────────────────────
  if ((cmd[0] == 'R' || cmd[0] == 'r') &&
      (cmd[1] == 'S' || cmd[1] == 's') &&
      (cmd[2] == 'T' || cmd[2] == 't')) {
    char motor = cmd[3];
    handleReset(motor);
    return;
  }
  
  // ─────────────────────────────────────────────────────────────
  // [BR] BIG REFERANS HIZ
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'B' || cmd[0] == 'b') && 
           (cmd[1] == 'R' || cmd[1] == 'r')) {
    if (cmd[2] == '\0') {
      handleBigRefShow();
    } else {
      handleBigRefAyarla(cmd);
    }
  }
  
  // ─────────────────────────────────────────────────────────────
  // [DC] DEPO ÇAPI (YENİ!)
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'D' || cmd[0] == 'd') && 
           (cmd[1] == 'C' || cmd[1] == 'c')) {
    if (cmd[2] == '\0') {
      handleDepoCapShow();
    } else {
      handleDepoCapAyarla(cmd);
    }
  }

  // ─────────────────────────────────────────────────────────────
  // [M] MOVETO
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'M' || cmd[0] == 'm') {
    char motor = cmd[1];
    uint8_t motorIndex;
    
    if (motor == 'Z' || motor == 'z') motorIndex = MOTOR_Z;
    else if (motor == 'X' || motor == 'x') motorIndex = MOTOR_X;
    else if (motor == 'B' || motor == 'b') motorIndex = MOTOR_B;
    else {
      Serial.println(F("✗ Geçersiz motor! (MZ/MX/MB)"));
      return;
    }
    
    long hedef;
    unsigned int hz;
    
    if (sscanf(cmd + 2, "%ld %u", &hedef, &hz) == 2) {
      if (hz == 0) {
        Serial.println(F("✗ Hz 0 olamaz!"));
        return;
      }
      
      Serial.print(F("[M"));
      Serial.print(motor);
      Serial.print(F("] "));
      Serial.print(hedef);
      Serial.print(F(" @ "));
      Serial.print(hz);
      Serial.print(F("Hz → "));
      
      if (moveTo(motorIndex, hedef, hz)) {
        Serial.println(F("✓"));
      } else {
        Serial.println(F("✗ (Aktif)"));
      }
    } else {
      Serial.println(F("✗ Format: MZ hedef hz"));
    }
  }
  
  // ─────────────────────────────────────────────────────────────
  // [D] DURDUR (MOTOR)
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'D' || cmd[0] == 'd') && 
           (cmd[1] == 'Z' || cmd[1] == 'z' || cmd[1] == 'X' || cmd[1] == 'x' || cmd[1] == 'B' || cmd[1] == 'b')) {
    char motor = cmd[1];
    uint8_t motorIndex;
    
    if (motor == 'Z' || motor == 'z') motorIndex = MOTOR_Z;
    else if (motor == 'X' || motor == 'x') motorIndex = MOTOR_X;
    else if (motor == 'B' || motor == 'b') motorIndex = MOTOR_B;
    else {
      Serial.println(F("✗ Geçersiz motor! (DZ/DX/DB)"));
      return;
    }
    
    pulseAtDurdur(motorIndex);
    moveToDurdur(motorIndex);
    
    Serial.print(F("[D"));
    Serial.print(motor);
    Serial.println(F("] ✓ Durdu"));
  }
  
  // ─────────────────────────────────────────────────────────────
  // [S] ACİL DURDURMA
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'S' || cmd[0] == 's') && (cmd[1] == '\0')) {
    Serial.println(F("\n⚠️  ACİL DURDURMA!"));
    pulseAtHepsiniDurdur();
    moveToHepsiniDurdur();
    kayitDurdur();
    oynatmaDurdur();
    ckDurdur();
    coDurdur();
    msDurdur();
    digitalWrite(KAYNAK_ROLE_PIN, HIGH);
    Serial.println(F("✓ Tüm sistemler durduruldu!\n"));
  }

  else if ((cmd[0] == 'S' || cmd[0] == 's') && (cmd[1] == 'M' || cmd[1] == 'm')) {
    Serial.println(F("\n⚠️  ACİL MOTOR DURDURMA!"));
    pulseAtHepsiniDurdur();
    moveToHepsiniDurdur();
    Serial.println(F("✓ Tüm motorlar durduruldu!\n"));
  }
  
  // ─────────────────────────────────────────────────────────────
  // [E] ENCODER/EXPORT KOMUTLARI
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'E' || cmd[0] == 'e') {
    
    // ESKİ EXPORT KOMUTLARI
    if (cmd[1] == '1') {
      ckExportStream1();
    }
    else if (cmd[1] == '2') {
      ckExportStream2();
    }
    else if (cmd[1] == '3') {
      ckExport3();
    }
    // ENCODER OKU
    else {
      handleEncoderOku();
    }
  }
  
  // ─────────────────────────────────────────────────────────────
  // [A] A0 SENSÖR
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'A' || cmd[0] == 'a') {
    handleA0Oku();
  }

  // ─────────────────────────────────────────────────────────────
  // [K] KAYNAK TOGGLE
  // ─────────────────────────────────────────────────────────────
  else if (strcmp(cmd, "K") == 0) {
    digitalWrite(KAYNAK_ROLE_PIN, !digitalRead(KAYNAK_ROLE_PIN));
    Serial.println(F("[KAYNAK] Toggle"));
  }
  
  // ─────────────────────────────────────────────────────────────
  // [H] HELP/MENU
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'H' || cmd[0] == 'h') {
    yazdirMenu();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [CK] ÇİFT KAYIT
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'C' || cmd[0] == 'c') && 
           (cmd[1] == 'K' || cmd[1] == 'k')) {
    handleCiftKayit();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [CO] ÇİFT OYNATMA
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'C' || cmd[0] == 'c') && 
           (cmd[1] == 'O' || cmd[1] == 'o')) {
    handleCiftOynatma();
  }
  
  // ═════════════════════════════════════════════════════════════
  // ✅ STREAM IMPORT KOMUTLARI (YENİ!)
  // ═════════════════════════════════════════════════════════════
  
  // ─────────────────────────────────────────────────────────────
  // [WS1] KAYIT1 STREAM IMPORT
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'W' && cmd[1] == '1' ) {
    ckImportStream1();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [WS2] KAYIT2 STREAM IMPORT
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'W' && cmd[1] == '2' ) {
    ckImportStream2();
  }
  
  
  
  // ─────────────────────────────────────────────────────────────
  // [W3] META DATA IMPORT
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'W' && cmd[1] == '3') {
    String veri = String(cmd).substring(2);
    veri.trim();
    
    if (veri.length() > 0) {
      ckImport3(veri);
    } else {
      Serial.println(F("✗ HATA: W3 komutundan sonra veri yok!"));
      Serial.println(F("  Kullanım: W3 <zRef> <x1> <x2> <globalA0Min> <globalA0Max>"));
    }
  }
  
  // ─────────────────────────────────────────────────────────────
  // [C1] KAYIT1 TEMİZLE
  // ─────────────────────────────────────────────────────────────
  else if (strcmp(cmd, "C1") == 0) {
    ckTemizle1();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [C2] KAYIT2 TEMİZLE
  // ─────────────────────────────────────────────────────────────
  else if (strcmp(cmd, "C2") == 0) {
    ckTemizle2();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [CA] HEPSİNİ TEMİZLE
  // ─────────────────────────────────────────────────────────────
  else if (strcmp(cmd, "CA") == 0) {
    ckHepsiniTemizle();
  }
  
  // ═════════════════════════════════════════════════════════════
  // OYNATMA DUR/DEVAM KOMUTLARI
  // ═════════════════════════════════════════════════════════════
  
  // ─────────────────────────────────────────────────────────────
  // [OP] OYNATMA PAUSE (Duraklat)
  // ─────────────────────────────────────────────────────────────
  else if (strcmp(cmd, "OP") == 0) {
    oynatmaDuraklat();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [OR] OYNATMA RESUME (Devam Et)
  // ─────────────────────────────────────────────────────────────
  else if (strcmp(cmd, "OR") == 0) {
    oynatmaDevamEt();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [X1] X1 POZİSYON AYARLA
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'X' || cmd[0] == 'x') && 
           (cmd[1] == '1')) {
    handleX1Ayarla(cmd);
  }
  
  // ─────────────────────────────────────────────────────────────
  // [X2] X2 POZİSYON AYARLA
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'X' || cmd[0] == 'x') && 
           (cmd[1] == '2')) {
    handleX2Ayarla(cmd);
  }
  
  // ─────────────────────────────────────────────────────────────
  // [X] X POZİSYONLARI GÖSTER
  // ─────────────────────────────────────────────────────────────
  else if ((cmd[0] == 'X' || cmd[0] == 'x') && 
           (cmd[1] == ' ' || cmd[1] == '\0')) {
    handleXShow();
  }
  
  // ─────────────────────────────────────────────────────────────
  // BİLİNMEYEN KOMUT
  // ─────────────────────────────────────────────────────────────
  else {
    Serial.print(F("✗ Bilinmeyen komut: "));
    Serial.println(cmd);
    Serial.println(F("  'H' yazın menüyü görmek için."));
  }
}

// ═══════════════════════════════════════════════════════════════
// YARDIMCI FONKSİYONLAR
// ═══════════════════════════════════════════════════════════════

void handleEncoderOku() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║              ENCODER POZİSYONLARI              ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.print(F("  Z (ENC2): "));
  Serial.println(zEnc.getPosition());
  
  Serial.print(F("  X (ENC1): "));
  Serial.println(xEnc.getPosition());
  
  Serial.print(F("  BIG (ENC3): "));
  Serial.println(bigEnc.getPosition());
  
  Serial.println();
}

void handleA0Oku() {
  uint16_t filtrelenmis = a0FiltreliOku();
  
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║                A0 SENSÖR                       ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.print(F("  Filtreli : "));
  Serial.println(filtrelenmis);
  
  Serial.println();
}

void handleBigRefAyarla(const char* cmd) {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║        BIG REFERANS HIZ AYARLAMA               ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));

  const char* arg = cmd + 2;
  while (*arg == ' ' || *arg == '\t') arg++;

  long yeniDeger;
  if (sscanf(arg, "%ld", &yeniDeger) == 1) {
    if (yeniDeger < 10 || yeniDeger > 500) {
      Serial.println(F("✗ Değer 10-500 arasında olmalı!"));
      Serial.println();
      return;
    }

    bigFreqRef = yeniDeger;

    Serial.println(F("✓ Referans hız güncellendi!"));
    Serial.print(F("  bigFreqRef = "));
    Serial.print(bigFreqRef);
    Serial.println(F(" Hz"));
  } else {
    Serial.println(F("✗ Geçersiz format!"));
    Serial.println(F("  Kullanım: BR 50 veya BR50"));
  }

  Serial.println();
}

void handleBigRefShow() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║         BIG REFERANS HIZ AYARI                 ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.print(F("  bigFreqRef: "));
  Serial.print(bigFreqRef);
  Serial.println(F(" Hz"));
  
  Serial.println(F("───────────────────────────────────────────────"));
  Serial.println(F("  Not: Depo kenarındaki (globalA0Min) hızdır."));
  Serial.println(F("       İçe doğru gidildikçe hız otomatik artar."));
  
  Serial.println();
}

void handleDepoCapAyarla(const char* cmd) {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║           DEPO ÇAPI AYARLAMA                   ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));

  const char* arg = cmd + 2;
  while (*arg == ' ' || *arg == '\t') arg++;

  float yeniDeger;
  if (sscanf(arg, "%f", &yeniDeger) == 1) {
    if (yeniDeger < 100.0 || yeniDeger > 2000.0) {
      Serial.println(F("✗ Değer 100-2000 mm arasında olmalı!"));
      Serial.println();
      return;
    }

    depoCapMm = yeniDeger;

    Serial.println(F("✓ Depo çapı güncellendi!"));
    Serial.print(F("  depoCapMm = "));
    Serial.print(depoCapMm);
    Serial.println(F(" mm"));
  } else {
    Serial.println(F("✗ Geçersiz format!"));
    Serial.println(F("  Kullanım: DC 520 veya DC 520.5"));
  }

  Serial.println();
}

void handleDepoCapShow() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║            DEPO ÇAPI AYARI                     ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.print(F("  depoCapMm: "));
  Serial.print(depoCapMm);
  Serial.println(F(" mm"));
  
  Serial.println(F("───────────────────────────────────────────────"));
  Serial.println(F("  Not: Depo kenarındaki (globalA0Min) çaptır."));
  Serial.println(F("       BIG motor hız hesaplamasında kullanılır."));
  
  Serial.println();
}

void handleCiftKayit() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║             ÇİFT KAYIT BAŞLATMA                ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.println(F("\nMevcut X Pozisyonları:"));
  Serial.print(F("  x1Pos = "));
  Serial.println(x1Pos);
  Serial.print(F("  x2Pos = "));
  Serial.println(x2Pos);
  Serial.println(F("───────────────────────────────────────────────"));
  
  ckBaslat(x1Pos, x2Pos, 0, 1);
}

void handleCiftOynatma() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║            ÇİFT OYNATMA BAŞLATMA               ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.println(F("\nMevcut Parametreler:"));
  Serial.print(F("  x1Pos     = "));
  Serial.println(x1Pos);
  Serial.print(F("  x2Pos     = "));
  Serial.println(x2Pos);
  Serial.print(F("  BigFreqMin= "));
  Serial.println(bigFreqMin);
  Serial.print(F("  BigFreqMax= "));
  Serial.println(bigFreqMax);
  Serial.print(F("  BigFreqRef= "));
  Serial.println(bigFreqRef);
  Serial.print(F("  DepoCap   = "));
  Serial.print(depoCapMm);
  Serial.println(F(" mm"));
  Serial.print(F("  zEncMin   = "));
  Serial.println(zEncMin);
  Serial.print(F("  zEncMax   = "));
  Serial.println(zEncMax);
  Serial.println(F("───────────────────────────────────────────────"));
  
  coBaslat(x1Pos, x2Pos);
}

void handleReset(char motor) {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║            ENCODER SIFIRLAMA                   ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));
  
  if (motor == 'Z' || motor == 'z') {
    long onceki = zEnc.getPosition();
    zEnc.reset();
    
    Serial.print(F("  Z Encoder sıfırlandı! (Önceki: "));
    Serial.print(onceki);
    Serial.println(F(")"));
  }
  else if (motor == 'X' || motor == 'x') {
    long onceki = xEnc.getPosition();
    xEnc.reset();
    
    Serial.print(F("  X Encoder sıfırlandı! (Önceki: "));
    Serial.print(onceki);
    Serial.println(F(")"));
  }
  else if (motor == 'B' || motor == 'b') {
    long onceki = bigEnc.getPosition();
    bigEnc.reset();
    
    Serial.print(F("  BIG Encoder sıfırlandı! (Önceki: "));
    Serial.print(onceki);
    Serial.println(F(")"));
  }
  else {
    Serial.println(F("✗ Geçersiz motor! (Z/X/B)"));
  }
  
  Serial.println();
}

void handleX1Ayarla(const char* cmd) {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║           X1 POZİSYON AYARLAMA                 ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));
  
  if (strstr(cmd, "SET") != nullptr || strstr(cmd, "set") != nullptr) {
    x1Pos = xEnc.getPosition();
    
    Serial.println(F("✓ X1 pozisyonu güncellendi!"));
    Serial.print(F("  x1Pos = "));
    Serial.println(x1Pos);
  }
  else {
    long yeniDeger;
    if (sscanf(cmd + 3, "%ld", &yeniDeger) == 1) {
      x1Pos = yeniDeger;
      
      Serial.println(F("✓ X1 pozisyonu güncellendi!"));
      Serial.print(F("  x1Pos = "));
      Serial.println(x1Pos);
    }
    else {
      Serial.println(F("✗ Geçersiz format!"));
      Serial.println(F("  Kullanım: X1 SET  veya  X1 5000"));
    }
  }
  
  Serial.println();
}

void handleX2Ayarla(const char* cmd) {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║           X2 POZİSYON AYARLAMA                 ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));
  
  if (strstr(cmd, "SET") != nullptr || strstr(cmd, "set") != nullptr) {
    x2Pos = xEnc.getPosition();
    
    Serial.println(F("✓ X2 pozisyonu güncellendi!"));
    Serial.print(F("  x2Pos = "));
    Serial.println(x2Pos);
  }
  else {
    long yeniDeger;
    if (sscanf(cmd + 3, "%ld", &yeniDeger) == 1) {
      x2Pos = yeniDeger;
      
      Serial.println(F("✓ X2 pozisyonu güncellendi!"));
      Serial.print(F("  x2Pos = "));
      Serial.println(x2Pos);
    }
    else {
      Serial.println(F("✗ Geçersiz format!"));
      Serial.println(F("  Kullanım: X2 SET  veya  X2 12000"));
    }
  }
  
  Serial.println();
}

void handleXShow() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║          X POZİSYON AYARLARI                   ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.print(F("  x1Pos (Kayıt1): "));
  Serial.println(x1Pos);
  
  Serial.print(F("  x2Pos (Kayıt2): "));
  Serial.println(x2Pos);
  
  Serial.println(F("───────────────────────────────────────────────"));
  Serial.print(F("  Mevcut X encoder: "));
  Serial.println(xEnc.getPosition());
  
  Serial.println();
}

// ═══════════════════════════════════════════════════════════════
// MENÜ YAZDIR - ✅ STREAM KOMUTLARI EKLENDİ
// ═══════════════════════════════════════════════════════════════
void yazdirMenu() {
  Serial.println(F("╔════════════════════════════════════════════════╗"));
  Serial.println(F("║                ANA MENÜ                        ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  Serial.println(F("  MZ/MX/MB <hedef> <hz> → Motor hareket"));
  Serial.println(F("  ─────────────────────────────────────────────"));
  Serial.println(F("  📊 KAYIT YÖNETİMİ:"));
  Serial.println(F("  W1                   → Kayıt1 stream import ⭐"));
  Serial.println(F("  W2                   → Kayıt2 stream import ⭐"));
  Serial.println(F("  W3 <meta>             → Meta data import"));
  Serial.println(F("  E1                   → Kayıt1 stream export ⭐"));
  Serial.println(F("  E2                   → Kayıt2 stream export ⭐"));
  Serial.println(F("  E3                    → Meta data export"));
  Serial.println(F("  C1                    → Kayıt1 temizle"));
  Serial.println(F("  C2                    → Kayıt2 temizle"));
  Serial.println(F("  CA                    → Hepsini temizle"));
  Serial.println(F("  ─────────────────────────────────────────────"));
  Serial.println(F("  ▶️ OYNATMA KONTROL:"));
  Serial.println(F("  OP                    → Oynatma duraklat"));
  Serial.println(F("  OR                    → Oynatma devam et"));
  Serial.println(F("  ─────────────────────────────────────────────"));
  Serial.println(F("  ⚙️ AYARLAR:"));
  Serial.println(F("  BR <değer> / BR       → Big referans hız =ç/hz=520/30oranı onerilir."));
  Serial.println(F("  DC <değer> / DC       → Depo çapı kısa kenar (mm)"));
  Serial.println(F("  X1 SET / X1 <değer>   → X1 pozisyon"));
  Serial.println(F("  X2 SET / X2 <değer>   → X2 pozisyon"));
  Serial.println(F("  X                     → X pozisyonlarını göster"));
  Serial.println(F("  ─────────────────────────────────────────────"));
  Serial.println(F("  K                     → Kaynak röle toggle"));
  Serial.println(F("  DZ/DX/DB              → Motor durdur"));
  Serial.println(F("  S                     → Acil durdur"));
  Serial.println(F("  SM                    → Motorları durdur"));
  Serial.println(F("  E                     → Encoder oku"));
  Serial.println(F("  A                     → A0 sensör oku"));
  Serial.println(F("  RSTZ/RSTX/RSTB        → Encoder sıfırla"));
  Serial.println(F("  CK                    → Çift kayıt başlat"));
  Serial.println(F("  CO                    → Çift oynatma başlat"));
  Serial.println(F("  H                     → Menü"));
  Serial.println(F("───────────────────────────────────────────────"));
  Serial.println(F("  ⭐ = Önerilen stream komutlar (buffer dostu)"));
  Serial.println(F("───────────────────────────────────────────────\n"));
}
