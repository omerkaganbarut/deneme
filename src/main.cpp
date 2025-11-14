// main.cpp - v10.0 TEMİZ VERSİYON + PUNTA MODU
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
  Serial.println(F("✓ Oynatma modülü hazır!\n"));
  
  Serial.println(F("[5/5] Çift Kayıt/Oynatma modülleri ayarlanıyor..."));
  ckEncoderSetup(&bigEnc, &xEnc);
  coEncoderSetup(&bigEnc, &xEnc, &zEnc);
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
    
    if (cmd[1] == '1') {
      ckExportStream1();
    }
    else if (cmd[1] == '2') {
      ckExportStream2();
    }
    else if (cmd[1] == '3') {
      ckExport3();
    }
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
  
  // ─────────────────────────────────────────────────────────────
  // [W1] KAYIT1 STREAM IMPORT
  // ─────────────────────────────────────────────────────────────
  else if (cmd[0] == 'W' && cmd[1] == '1' ) {
    ckImportStream1();
  }
  
  // ─────────────────────────────────────────────────────────────
  // [W2] KAYIT2 STREAM IMPORT
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
      Serial.println(F("  Kullanım: W3 <zRef> <x1> <x2> <bigFreqRef> <depoCapMm>"));
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
  // [PM] PUNTA MODU TOGGLE
  // ─────────────────────────────────────────────────────────────
  else if (strcmp(cmd, "PM") == 0) {
    bool aktif = !oynatmaPuntaModuAktifMi();
    oynatmaPuntaModuAyarla(aktif);
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

void handleCiftKayit() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║             ÇİFT KAYIT BAŞLATMA                ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  Serial.println(F("\nParametreleri girin:"));
  Serial.println(F("Format: x1Pos x2Pos bigFreqRef depoCapMm"));
  Serial.println(F("Örnek: 0 -10000 30 520"));
  Serial.print(F("> "));
  
  // Kullanıcı girişini bekle
  while (!Serial.available()) {}
  
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  long x1, x2;
  float freq, cap;
  
  if (sscanf(input.c_str(), "%ld %ld %f %f", &x1, &x2, &freq, &cap) == 4) {
    // Meta verilere kaydet
    ckMeta.x1Pos = x1;
    ckMeta.x2Pos = x2;
    ckMeta.bigFreqRef = freq;
    ckMeta.depoCapMm = cap;
    
    Serial.println(F("\n✓ Parametreler ayarlandı:"));
    Serial.print(F("  x1Pos      = ")); Serial.println(x1);
    Serial.print(F("  x2Pos      = ")); Serial.println(x2);
    Serial.print(F("  bigFreqRef = ")); Serial.println(freq);
    Serial.print(F("  depoCapMm  = ")); Serial.println(cap);
    
    ckBaslat(x1, x2, 0, 1);
  } else {
    Serial.println(F("\n✗ Geçersiz format!"));
  }
}

void handleCiftOynatma() {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║            ÇİFT OYNATMA BAŞLATMA               ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  
  // W3'ten gelen parametreleri göster
  Serial.println(F("\nW3'ten Yüklenen Parametreler:"));
  Serial.print(F("  x1Pos      = ")); Serial.println(ckMeta.x1Pos);
  Serial.print(F("  x2Pos      = ")); Serial.println(ckMeta.x2Pos);
  Serial.print(F("  zRefPos    = ")); Serial.println(ckMeta.zRefPos);
  Serial.print(F("  bigFreqRef = ")); Serial.println(ckMeta.bigFreqRef);
  Serial.print(F("  depoCapMm  = ")); Serial.println(ckMeta.depoCapMm);
  
  uint16_t a0Min, a0Max;
  ckHesaplaGlobalA0MinMax(&a0Min, &a0Max);
  Serial.print(F("  globalA0Min= ")); Serial.println(a0Min);
  Serial.print(F("  globalA0Max= ")); Serial.println(a0Max);
  
  Serial.println(F("───────────────────────────────────────────────"));
  
  coBaslat(ckMeta.x1Pos, ckMeta.x2Pos);
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

// ═══════════════════════════════════════════════════════════════
// MENÜ YAZDIR
// ═══════════════════════════════════════════════════════════════
void yazdirMenu() {
  Serial.println(F("╔════════════════════════════════════════════════╗"));
  Serial.println(F("║                ANA MENÜ                        ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝"));
  Serial.println(F("  MZ/MX/MB <hedef> <hz> → Motor hareket"));
  Serial.println(F("  ─────────────────────────────────────────────"));
  Serial.println(F("  📊 KAYIT YÖNETİMİ:"));
  Serial.println(F("  W1                   → Kayıt1 stream import"));
  Serial.println(F("  W2                   → Kayıt2 stream import"));
  Serial.println(F("  W3 <meta>            → Meta data import"));
  Serial.println(F("  E1                   → Kayıt1 stream export"));
  Serial.println(F("  E2                   → Kayıt2 stream export"));
  Serial.println(F("  E3                   → Meta data export"));
  Serial.println(F("  C1/C2/CA             → Kayıtları temizle"));
  Serial.println(F("  ─────────────────────────────────────────────"));
  Serial.println(F("  ▶️ OYNATMA KONTROL:"));
  Serial.println(F("  OP                   → Oynatma duraklat"));
  Serial.println(F("  OR                   → Oynatma devam et"));
  Serial.println(F("  PM                   → Punta modu toggle ⭐"));
  Serial.println(F("  ─────────────────────────────────────────────"));
  Serial.println(F("  K                    → Kaynak röle toggle"));
  Serial.println(F("  DZ/DX/DB             → Motor durdur"));
  Serial.println(F("  S / SM               → Acil durdur"));
  Serial.println(F("  E / A                → Encoder/A0 oku"));
  Serial.println(F("  RSTZ/RSTX/RSTB       → Encoder sıfırla"));
  Serial.println(F("  CK                   → Çift kayıt başlat ⭐"));
  Serial.println(F("  CO                   → Çift oynatma başlat"));
  Serial.println(F("  H                    → Menü"));
  Serial.println(F("───────────────────────────────────────────────"));
  Serial.println(F("  ⭐ = Yeni özellikler (Punta + Parametreli CK)"));
  Serial.println(F("───────────────────────────────────────────────\n"));
}