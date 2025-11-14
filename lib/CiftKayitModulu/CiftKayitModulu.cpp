// CiftKayitModulu.cpp - v9.0 TEMİZ VERSİYON
#include "CiftKayitModulu.h"
#include "Config.h"
#include "KayitModulu.h"
#include "MoveTo.h"
#include "CiftOynatmaModulu.h"

// ═══════════════════════════════════════════════════════════════
// DURUM MAKİNESİ
// ═══════════════════════════════════════════════════════════════
enum CKDurum {
  CK_KAPALI = 0,
  CK_X1_GIDIYOR,
  CK_X1_ONAY_BEKLE,
  CK_KAYIT1_CALISIYOR,
  CK_X2_GIDIYOR,
  CK_X2_ONAY_BEKLE,
  CK_KAYIT2_CALISIYOR,
  CK_TAMAMLANDI
};

// ═══════════════════════════════════════════════════════════════
// STATİK DEĞİŞKENLER
// ═══════════════════════════════════════════════════════════════
static CKDurum durum = CK_KAPALI;
static StepMotorEncoder* bigEnc = nullptr;
static StepMotorEncoder* xEnc = nullptr;
static long x1Hedef = 0;
static long x2Hedef = 0;
static int yon1 = 0;
static int yon2 = 1;

// ═══════════════════════════════════════════════════════════════
// GLOBAL TANIMLAR
// ═══════════════════════════════════════════════════════════════
Sample kayit1[KAYIT_ORNEK_SAYISI];
Sample kayit2[KAYIT_ORNEK_SAYISI];
CK_MetaData ckMeta = {0, 0, 0, 30.0, 520.0};

// ═══════════════════════════════════════════════════════════════
// ENCODER SETUP
// ═══════════════════════════════════════════════════════════════
void ckEncoderSetup(StepMotorEncoder* bigEncoder, StepMotorEncoder* xEncoder) {
  bigEnc = bigEncoder;
  xEnc = xEncoder;
}

// ═══════════════════════════════════════════════════════════════
// GLOBAL A0 MIN/MAX HESAPLAMA
// ═══════════════════════════════════════════════════════════════
void ckHesaplaGlobalA0MinMax(uint16_t* outMin, uint16_t* outMax) {
  uint16_t minVal = 1023;
  uint16_t maxVal = 0;
  
  // Kayıt1'den tara
  for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
    if (kayit1[i].a0 < minVal) minVal = kayit1[i].a0;
    if (kayit1[i].a0 > maxVal) maxVal = kayit1[i].a0;
  }
  
  // Kayıt2'den tara
  for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
    if (kayit2[i].a0 < minVal) minVal = kayit2[i].a0;
    if (kayit2[i].a0 > maxVal) maxVal = kayit2[i].a0;
  }
  
  *outMin = minVal;
  *outMax = maxVal;
}

// ═══════════════════════════════════════════════════════════════
// ÇİFT KAYIT BAŞLATMA
// ═══════════════════════════════════════════════════════════════
void ckBaslat(long x1Enc, long x2Enc, int kayit1Yon, int kayit2Yon) {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║           ÇİFT KAYIT BAŞLATILIYOR              ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));
  
  coZSifirlamaReset();
  
  // Meta verileri hazırla
  ckMeta.x1Pos = x1Enc;
  ckMeta.x2Pos = x2Enc;
  ckMeta.zRefPos = 0;
  
  x1Hedef = x1Enc;
  x2Hedef = x2Enc;
  yon1 = kayit1Yon;
  yon2 = kayit2Yon;
  
  Serial.print(F("  X1 Pozisyonu: "));
  Serial.print(x1Hedef);
  Serial.print(F(" (Yön: "));
  Serial.print(yon1 ? F("Geri") : F("İleri"));
  Serial.println(F(")"));
  
  Serial.print(F("  X2 Pozisyonu: "));
  Serial.print(x2Hedef);
  Serial.print(F(" (Yön: "));
  Serial.print(yon2 ? F("Geri") : F("İleri"));
  Serial.println(F(")"));
  
  if (bigEnc == nullptr || xEnc == nullptr) {
    Serial.println(F("\n✗ Hata: Encoder'lar ayarlanmamış!"));
    return;
  }
  
  Serial.println(F("\n[ADIM 1/6] X1 pozisyonuna gidiliyor..."));
  moveTo(MOTOR_X, x1Hedef, 10000, false);
  durum = CK_X1_GIDIYOR;
}

// ═══════════════════════════════════════════════════════════════
// ÇİFT KAYIT ARKA PLAN
// ═══════════════════════════════════════════════════════════════
void ckRun() {
  switch (durum) {
    
    case CK_KAPALI:
      return;
    
    case CK_X1_GIDIYOR:
      if (!moveToAktifMi(MOTOR_X)) {
        Serial.println(F("✓ X1 pozisyonuna ulaşıldı!\n"));
        Serial.print(F("  Mevcut X: "));
        Serial.println(xEnc->getPosition());
        
        Serial.println(F("\n[ADIM 2/6] Kayıt1 başlatmaya hazır."));
        Serial.println(F("───────────────────────────────────────────────"));
        Serial.println(F("  Kayıt1'i başlatmak için 'Y' tuşuna basın."));
        Serial.println(F("  İptal için 'N' tuşuna basın."));
        Serial.print(F("  > "));
        
        durum = CK_X1_ONAY_BEKLE;
      }
      break;
    
    case CK_X1_ONAY_BEKLE:
      if (Serial.available() > 0) {
        char c = Serial.peek();
        
        if (c == 'Y' || c == 'y') {
          Serial.read();
          Serial.println(F("Y\n"));
          
          Serial.println(F("[ADIM 3/6] Kayıt1 başlatılıyor...\n"));
          kayitBaslat(yon1);
          
          durum = CK_KAYIT1_CALISIYOR;
        }
        else if (c == 'N' || c == 'n') {
          Serial.read();
          Serial.println(F("N\n"));
          Serial.println(F("✗ Çift kayıt iptal edildi!"));
          
          durum = CK_KAPALI;
        }
      }
      break;
    
    case CK_KAYIT1_CALISIYOR:
      if (kayitTamamlandiMi()) {
        Serial.println(F("\n✓ Kayıt1 tamamlandı!\n"));
        
        // KAYIT1'İ KOPYALA
        const Sample* src = kayitVerileri();
        for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
          kayit1[i].enc = src[i].enc;
          kayit1[i].a0 = src[i].a0;
        }
        
        Serial.println(F("→ Kayıt1 kaydedildi."));
        Serial.print(F("  Örnek sayısı: "));
        Serial.println(kayitOrnekSayisi());
        
        Serial.println(F("\n[ADIM 4/6] X2 pozisyonuna gidiliyor..."));
        
        moveTo(MOTOR_X, x2Hedef, 10000, false);
        durum = CK_X2_GIDIYOR;
      }
      break;
    
    case CK_X2_GIDIYOR:
      if (!moveToAktifMi(MOTOR_X)) {
        Serial.println(F("✓ X2 pozisyonuna ulaşıldı!\n"));
        Serial.print(F("  Mevcut X: "));
        Serial.println(xEnc->getPosition());
        
        Serial.println(F("\n[ADIM 5/6] Kayıt2 başlatmaya hazır."));
        Serial.println(F("───────────────────────────────────────────────"));
        Serial.println(F("  Kayıt2'yi başlatmak için 'Y' tuşuna basın."));
        Serial.println(F("  İptal için 'N' tuşuna basın."));
        Serial.print(F("  > "));
        
        durum = CK_X2_ONAY_BEKLE;
      }
      break;
    
    case CK_X2_ONAY_BEKLE:
      if (Serial.available() > 0) {
        char c = Serial.peek();
        
        if (c == 'Y' || c == 'y') {
          Serial.read();
          Serial.println(F("Y\n"));
          
          Serial.println(F("[ADIM 6/6] Kayıt2 başlatılıyor...\n"));
          kayitBaslat(yon2);
          
          durum = CK_KAYIT2_CALISIYOR;
        }
        else if (c == 'N' || c == 'n') {
          Serial.read();
          Serial.println(F("N\n"));
          Serial.println(F("✗ Kayıt2 iptal edildi!"));
          
          durum = CK_KAPALI;
        }
      }
      break;
    
    case CK_KAYIT2_CALISIYOR:
      if (kayitTamamlandiMi()) {
        Serial.println(F("\n✓ Kayıt2 tamamlandı!\n"));
        
        // KAYIT2'Yİ KOPYALA
        const Sample* src = kayitVerileri();
        for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
          kayit2[i].enc = src[i].enc;
          kayit2[i].a0 = src[i].a0;
        }
        
        Serial.println(F("→ Kayıt2 kaydedildi."));
        Serial.print(F("  Örnek sayısı: "));
        Serial.println(kayitOrnekSayisi());
        
        // Global A0 hesapla ve göster
        uint16_t a0Min, a0Max;
        ckHesaplaGlobalA0MinMax(&a0Min, &a0Max);
        
        Serial.println(F("\n╔════════════════════════════════════════════════╗"));
        Serial.println(F("║          ÇİFT KAYIT TAMAMLANDI! ✓              ║"));
        Serial.println(F("╚════════════════════════════════════════════════╝\n"));
        
        Serial.println(F("─────────────────────────────────────────────────"));
        Serial.println(F("GLOBAL A0 ARALIĞI:"));
        Serial.print(F("  Min   : "));
        Serial.println(a0Min);
        Serial.print(F("  Max   : "));
        Serial.println(a0Max);
        Serial.print(F("  Aralık: "));
        Serial.println(a0Max - a0Min);
        Serial.println(F("─────────────────────────────────────────────────\n"));
        
        Serial.println(F("✓ Çift oynatma için hazır!"));
        Serial.println(F("  Komut: CO (Çift Oynatma)\n"));
        
        durum = CK_TAMAMLANDI;
      }
      break;
    
    case CK_TAMAMLANDI:
      return;
    
    default:
      Serial.println(F("✗ Bilinmeyen durum!"));
      durum = CK_KAPALI;
      break;
  }
}

// ═══════════════════════════════════════════════════════════════
// DURUM FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

bool ckAktifMi() {
  return (durum != CK_KAPALI && durum != CK_TAMAMLANDI);
}

bool ckTamamlandiMi() {
  return (durum == CK_TAMAMLANDI);
}

void ckDurdur() {
  Serial.println(F("\n✗ Çift kayıt durduruldu!"));
  
  kayitDurdur();
  moveToDurdur(MOTOR_X);
  
  durum = CK_KAPALI;
}

// ═══════════════════════════════════════════════════════════════
// EXPORT FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

void ckExport3() {
  Serial.print(F("W3 "));
  Serial.print(ckMeta.zRefPos);
  Serial.print(F(" "));
  Serial.print(ckMeta.x1Pos);
  Serial.print(F(" "));
  Serial.print(ckMeta.x2Pos);
  Serial.print(F(" "));
  Serial.print(ckMeta.bigFreqRef, 1);
  Serial.print(F(" "));
  Serial.print(ckMeta.depoCapMm, 1);
  
  Serial.println();
  
  Serial.println(F("\n✅ Meta data export edildi!"));
  Serial.print(F("   zRefPos    : ")); Serial.println(ckMeta.zRefPos);
  Serial.print(F("   x1Pos      : ")); Serial.println(ckMeta.x1Pos);
  Serial.print(F("   x2Pos      : ")); Serial.println(ckMeta.x2Pos);
  Serial.print(F("   bigFreqRef : ")); Serial.println(ckMeta.bigFreqRef);
  Serial.print(F("   depoCapMm  : ")); Serial.println(ckMeta.depoCapMm);
  Serial.println(F("   Yukarıdaki 'W3 ...' satırını kopyalayabilirsin."));
}

// ═══════════════════════════════════════════════════════════════
// IMPORT FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

bool ckImport3(String veriStr) {
  Serial.println(F("\n╔════════════════════════════════════════════════╗"));
  Serial.println(F("║           META DATA IMPORT BAŞLIYOR            ║"));
  Serial.println(F("╚════════════════════════════════════════════════╝\n"));
  
  if (veriStr.length() == 0) {
    Serial.println(F("✗ HATA: Veri yok!"));
    return false;
  }
  
  int start = 0;
  int end;
  
  // zRefPos
  end = veriStr.indexOf(' ', start);
  if (end == -1) { Serial.println(F("✗ HATA: zRefPos yok!")); return false; }
  ckMeta.zRefPos = veriStr.substring(start, end).toInt();
  start = end + 1;
  
  // x1Pos
  end = veriStr.indexOf(' ', start);
  if (end == -1) { Serial.println(F("✗ HATA: x1Pos yok!")); return false; }
  ckMeta.x1Pos = veriStr.substring(start, end).toInt();
  start = end + 1;
  
  // x2Pos
  end = veriStr.indexOf(' ', start);
  if (end == -1) { Serial.println(F("✗ HATA: x2Pos yok!")); return false; }
  ckMeta.x2Pos = veriStr.substring(start, end).toInt();
  start = end + 1;
  
  // bigFreqRef
  end = veriStr.indexOf(' ', start);
  if (end == -1) { Serial.println(F("✗ HATA: bigFreqRef yok!")); return false; }
  ckMeta.bigFreqRef = veriStr.substring(start, end).toFloat();
  start = end + 1;
  
  // depoCapMm
  String capStr = veriStr.substring(start);
  capStr.trim();
  if (capStr.length() == 0) { Serial.println(F("✗ HATA: depoCapMm yok!")); return false; }
  ckMeta.depoCapMm = capStr.toFloat();
  
  // DURUM GÜNCELLEMESİ
  durum = CK_TAMAMLANDI;
  
  // Global A0 hesapla ve göster
  uint16_t a0Min, a0Max;
  ckHesaplaGlobalA0MinMax(&a0Min, &a0Max);
  
  Serial.println(F("✅ META DATA IMPORT TAMAMLANDI!"));
  Serial.println(F("─────────────────────────────────────────────"));
  Serial.print(F("  zRefPos    : ")); Serial.println(ckMeta.zRefPos);
  Serial.print(F("  x1Pos      : ")); Serial.println(ckMeta.x1Pos);
  Serial.print(F("  x2Pos      : ")); Serial.println(ckMeta.x2Pos);
  Serial.print(F("  bigFreqRef : ")); Serial.println(ckMeta.bigFreqRef);
  Serial.print(F("  depoCapMm  : ")); Serial.println(ckMeta.depoCapMm);
  Serial.print(F("  globalA0Min: ")); Serial.println(a0Min);
  Serial.print(F("  globalA0Max: ")); Serial.println(a0Max);
  Serial.print(F("  A0 Aralık  : ")); Serial.println(a0Max - a0Min);
  Serial.println(F("─────────────────────────────────────────────"));
  
  Serial.println(F("\n✅ TÜM VERİLER YÜKLENDİ!"));
  Serial.println(F("✓ CO komutuyla oynatma başlatılabilir!"));
  Serial.println();
  
  return true;
}

// ═══════════════════════════════════════════════════════════════
// TEMİZLEME FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

void ckTemizle1() {
  for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
    kayit1[i].enc = 0;
    kayit1[i].a0 = 0;
  }
  Serial.println(F("✅ Kayit1 temizlendi!"));
}

void ckTemizle2() {
  for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
    kayit2[i].enc = 0;
    kayit2[i].a0 = 0;
  }
  Serial.println(F("✅ Kayit2 temizlendi!"));
}

void ckHepsiniTemizle() {
  ckTemizle1();
  ckTemizle2();
  ckMeta.zRefPos = 0;
  ckMeta.x1Pos = 0;
  ckMeta.x2Pos = 0;
  ckMeta.bigFreqRef = 30.0;
  ckMeta.depoCapMm = 520.0;
  durum = CK_KAPALI;
  Serial.println(F("✅ Tüm kayıtlar ve meta veriler temizlendi!"));
}

// ═══════════════════════════════════════════════════════════════
// STREAM IMPORT FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

bool ckImportStream1() {
  Serial.println(F("\n╔═══════════════════════════════════════════╗"));
  Serial.println(F("║      KAYIT1 STREAM IMPORT BAŞLIYOR        ║"));
  Serial.println(F("╚═══════════════════════════════════════════╝"));
  Serial.println(F("Format: enc,a0 enc,a0 enc,a0 ..."));
  Serial.println(F("Bitirmek için: END"));
  Serial.println(F("Veri göndermeye başlayın:\n"));
  
  uint16_t idx = 0;
  String buffer = "";
  buffer.reserve(20);
  
  while (idx < KAYIT_ORNEK_SAYISI) {
    while (Serial.available() > 0) {
      char c = Serial.read();
      
      if (c == ' ' || c == '\n' || c == '\r') {
        if (buffer.length() > 0) {
          if (buffer.equals("END")) {
            Serial.println(F("\n\n✓ Stream tamamlandı!"));
            Serial.print(F("Yüklenen örnek: ")); Serial.println(idx);
            return true;
          }
          
          int virPos = buffer.indexOf(',');
          if (virPos > 0 && virPos < buffer.length() - 1) {
            long enc = buffer.substring(0, virPos).toInt();
            uint16_t a0 = buffer.substring(virPos + 1).toInt();
            
            kayit1[idx].enc = enc;
            kayit1[idx].a0 = a0;
            idx++;
            
            if (idx % 10 == 0) {
              Serial.print(F("."));
              if (idx % 100 == 0) {
                Serial.print(F(" ")); Serial.println(idx);
              }
            }
          }
          buffer = "";
        }
      }
      else {
        buffer += c;
        if (buffer.length() > 50) {
          Serial.println(F("\n✗ Format hatası! Buffer overflow!"));
          buffer = "";
        }
      }
    }
  }
  
  Serial.println(F("\n✓ Tüm örnekler alındı!"));
  Serial.print(F("Toplam: ")); Serial.println(idx);
  return true;
}

bool ckImportStream2() {
  Serial.println(F("\n╔═══════════════════════════════════════════╗"));
  Serial.println(F("║      KAYIT2 STREAM IMPORT BAŞLIYOR        ║"));
  Serial.println(F("╚═══════════════════════════════════════════╝"));
  Serial.println(F("Format: enc,a0 enc,a0 enc,a0 ..."));
  Serial.println(F("Bitirmek için: END"));
  Serial.println(F("Veri göndermeye başlayın:\n"));
  
  uint16_t idx = 0;
  String buffer = "";
  buffer.reserve(20);
  
  while (idx < KAYIT_ORNEK_SAYISI) {
    while (Serial.available() > 0) {
      char c = Serial.read();
      
      if (c == ' ' || c == '\n' || c == '\r') {
        if (buffer.length() > 0) {
          if (buffer.equals("END")) {
            Serial.println(F("\n\n✓ Stream tamamlandı!"));
            Serial.print(F("Yüklenen örnek: ")); Serial.println(idx);
            return true;
          }
          
          int virPos = buffer.indexOf(',');
          if (virPos > 0 && virPos < buffer.length() - 1) {
            long enc = buffer.substring(0, virPos).toInt();
            uint16_t a0 = buffer.substring(virPos + 1).toInt();
            
            kayit2[idx].enc = enc;
            kayit2[idx].a0 = a0;
            idx++;
            
            if (idx % 10 == 0) {
              Serial.print(F("."));
              if (idx % 100 == 0) {
                Serial.print(F(" ")); Serial.println(idx);
              }
            }
          }
          buffer = "";
        }
      }
      else {
        buffer += c;
        if (buffer.length() > 50) {
          Serial.println(F("\n✗ Format hatası!"));
          buffer = "";
        }
      }
    }
  }
  
  Serial.println(F("\n✓ Tüm örnekler alındı!"));
  Serial.print(F("Toplam: ")); Serial.println(idx);
  return true;
}

// ═══════════════════════════════════════════════════════════════
// STREAM EXPORT FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

void ckExportStream1() {
  Serial.println(F("\n╔═══════════════════════════════════════════╗"));
  Serial.println(F("║      KAYIT1 STREAM EXPORT                 ║"));
  Serial.println(F("╚═══════════════════════════════════════════╝"));
  Serial.println(F("W1 "));
  
  for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
    Serial.print(kayit1[i].enc);
    Serial.print(F(","));
    Serial.print(kayit1[i].a0);
    
    if (i < KAYIT_ORNEK_SAYISI - 1) {
      Serial.print(F(" "));
    }
    
    if ((i + 1) % 10 == 0) {
      Serial.print(F("."));
      if ((i + 1) % 100 == 0) {
        Serial.println();
      }
    }
  }
  
  Serial.println(F(" END"));
  Serial.println(F("\n✓ Export tamamlandı!"));
}

void ckExportStream2() {
  Serial.println(F("\n╔═══════════════════════════════════════════╗"));
  Serial.println(F("║      KAYIT2 STREAM EXPORT                 ║"));
  Serial.println(F("╚═══════════════════════════════════════════╝"));
  Serial.println(F("W2 "));
  
  for (uint16_t i = 0; i < KAYIT_ORNEK_SAYISI; i++) {
    Serial.print(kayit2[i].enc);
    Serial.print(F(","));
    Serial.print(kayit2[i].a0);
    
    if (i < KAYIT_ORNEK_SAYISI - 1) {
      Serial.print(F(" "));
    }
    
    if ((i + 1) % 10 == 0) {
      Serial.print(F("."));
      if ((i + 1) % 100 == 0) {
        Serial.println();
      }
    }
  }
  
  Serial.println(F(" END"));
  Serial.println(F("\n✓ Export tamamlandı!"));
}