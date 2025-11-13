// OynatmaModulu.cpp - v7.0 DİNAMİK DEPO ÇAPI + DUR/DEVAM
#include "OynatmaModulu.h"
#include "Config.h"
#include "CiftKayitModulu.h"
#include "PulseAt.h"
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
// DURUM MAKİNESİ
// ═══════════════════════════════════════════════════════════════
enum OynatmaDurum {
  OY_KAPALI = 0,
  OY_SEGMENT_OYNAT,
  OY_DURAKLATILDI,
  OY_TAMAMLANDI
};

// ═══════════════════════════════════════════════════════════════
// STATİK DEĞİŞKENLER
// ═══════════════════════════════════════════════════════════════
static OynatmaDurum durum = OY_KAPALI;
static uint16_t idx = 0;

static StepMotorEncoder* bigEnc = nullptr;
static StepMotorEncoder* zEnc = nullptr;

static long* bigFreqMinPtr = nullptr;
static long* bigFreqMaxPtr = nullptr;
static long* bigFreqRefPtr = nullptr;
static float* depoCapMmPtr = nullptr;  // ✅ YENİ: Dinamik depo çapı

static const Sample* kayitPtr = nullptr;
static uint16_t kayitOrnekSayisi = 0;

// ═══════════════════════════════════════════════════════════════
// ENCODER SETUP
// ═══════════════════════════════════════════════════════════════
void oynatmaEncoderSetup(StepMotorEncoder* bigEncoder, StepMotorEncoder* zEncoder) {
  bigEnc = bigEncoder;
  zEnc = zEncoder;
}

// ═══════════════════════════════════════════════════════════════
// PARAMETRE SETUP
// ═══════════════════════════════════════════════════════════════
void oynatmaParametreSetup(long* bigFreqMin, long* bigFreqMax, 
                           long* zEncMin, long* zEncMax) {
  bigFreqMinPtr = bigFreqMin;
  bigFreqMaxPtr = bigFreqMax;
  (void)zEncMin;
  (void)zEncMax;
}

void oynatmaRefHizSetup(long* bigFreqRefPtrArg) {
  bigFreqRefPtr = bigFreqRefPtrArg;
}

void oynatmaDepoCapSetup(float* depoCapMm) {
  depoCapMmPtr = depoCapMm;
}

// ═══════════════════════════════════════════════════════════════
// PUBLIC HELPER FONKSİYONLAR
// ═══════════════════════════════════════════════════════════════

long oynatmaHesaplaZMaxOffset() {
  if (globalA0Max <= globalA0Min) return Z_ENCODER_MAX;
  
  uint16_t a0Aralik = globalA0Max - globalA0Min;
  float oran = (float)a0Aralik / 1023.0;
  long zMaxOffset = (long)(oran * Z_ENCODER_MAX);
  
  if (zMaxOffset > Z_ENCODER_MAX) zMaxOffset = Z_ENCODER_MAX;
  if (zMaxOffset < 1000) zMaxOffset = 1000;
  
  return zMaxOffset;
}

long oynatmaMapA0ToZOffset(uint16_t a0) {
  if (a0 <= globalA0Min) return 0;
  
  long zMaxOffset = oynatmaHesaplaZMaxOffset();
  
  if (a0 >= globalA0Max) return zMaxOffset;
  
  return map(a0, globalA0Min, globalA0Max, 0, zMaxOffset);
}

long oynatmaMapA0ToZEnc(uint16_t a0) {
  long offset = oynatmaMapA0ToZOffset(a0);
  return ckMeta.zRefPos + offset;
}

unsigned int oynatmaMapA0ToBigFreq(uint16_t a0) {
  if (bigFreqMinPtr == nullptr || bigFreqMaxPtr == nullptr) return 100;
  if (bigFreqRefPtr == nullptr) return 100;
  if (depoCapMmPtr == nullptr) return 100;
  
  float mmPerA0 = A0_FIZIKSEL_ARALIK_MM / 1023.0;
  float yaricapRef = (*depoCapMmPtr) / 2.0;  // ✅ Dinamik depo çapı kullan
  float deltaYaricapMM = (a0 - globalA0Min) * mmPerA0;
  float yaricapMM = yaricapRef + deltaYaricapMM;
  
  float sabitCarpim = yaricapRef * (*bigFreqRefPtr);
  float freq = sabitCarpim / yaricapMM;
  
  if (freq < 10) freq = 10;
  
  return (unsigned int)freq;
}

// ═══════════════════════════════════════════════════════════════
// OYNATMA BAŞLATMA
// ═══════════════════════════════════════════════════════════════
void oynatmaBaslatKayit(const Sample* kayit, uint16_t ornekSayisi) {
  Serial.println(F("\n[OYNATMA] Başlatılıyor..."));
  
  if (kayit == nullptr) {
    Serial.println(F("✗ Kayıt pointer hatası!"));
    return;
  }
  
  if (ornekSayisi == 0) {
    Serial.println(F("✗ Örnek sayısı sıfır!"));
    return;
  }
  
  if (bigEnc == nullptr || zEnc == nullptr) {
    Serial.println(F("✗ Encoder hatası!"));
    return;
  }
  
  if (bigFreqMinPtr == nullptr || bigFreqMaxPtr == nullptr) {
    Serial.println(F("✗ Parametre hatası!"));
    return;
  }
  
  kayitPtr = kayit;
  kayitOrnekSayisi = ornekSayisi;
  
  Serial.print(F("  Global A0 Min: ")); Serial.println(globalA0Min);
  Serial.print(F("  Global A0 Max: ")); Serial.println(globalA0Max);
  Serial.print(F("  A0 Aralığı: ")); Serial.println(globalA0Max - globalA0Min);
  Serial.print(F("  Z Referans: ")); Serial.println(ckMeta.zRefPos);
  Serial.print(F("  Hesaplanan Z Max Offset: ")); Serial.println(oynatmaHesaplaZMaxOffset());
  
  if (depoCapMmPtr != nullptr) {
    Serial.print(F("  Depo Çapı: ")); Serial.print(*depoCapMmPtr); Serial.println(F(" mm"));
  }
  
  Serial.print(F("  Örnek sayısı: ")); Serial.println(ornekSayisi);
  
  idx = 0;
  durum = OY_SEGMENT_OYNAT;
  digitalWrite(KAYNAK_ROLE_PIN, LOW);
  
  Serial.println(F("[OYNATMA] Segment oynatma başladı!"));
}

// ═══════════════════════════════════════════════════════════════
// OYNATMA ARKA PLAN
// ═══════════════════════════════════════════════════════════════
void oynatmaRun() {
  if (durum == OY_KAPALI || durum == OY_TAMAMLANDI || durum == OY_DURAKLATILDI) return;
  if (kayitPtr == nullptr) return;
  
  switch (durum) {
    
    case OY_SEGMENT_OYNAT:
    {
      if (!pulseAtAktifMi(MOTOR_B) && !pulseAtAktifMi(MOTOR_Z)) {
        
        if (idx >= kayitOrnekSayisi - 1) {
          Serial.println(F("[OYNATMA] ✓ Tamamlandı!"));
          digitalWrite(KAYNAK_ROLE_PIN, HIGH);
          durum = OY_TAMAMLANDI;
          return;
        }
        
        uint16_t a0Start = kayitPtr[idx].a0;
        uint16_t a0End = kayitPtr[idx + 1].a0;
        
        // BIG MOTOR
        long bigTarget = kayitPtr[idx + 1].enc;
        long bigNow = bigEnc->getPosition();
        long dBig = bigTarget - bigNow;
        unsigned long masterPulses = abs(dBig);
        int masterYon = (dBig > 0) ? 0 : 1;
        unsigned int masterFreq = oynatmaMapA0ToBigFreq(a0Start);
        
        // Z MOTOR
        long zHedef = oynatmaMapA0ToZEnc(a0End);
        long zNow = zEnc->getPosition();
        long dZ = zHedef - zNow;
        unsigned long slavePulses = abs(dZ);
        int slaveYon = (dZ > 0) ? 0 : 1;
        
        unsigned int slaveFreq = 0;
        if (masterPulses > 0 && slavePulses > 0) {
          double fsd = (double)slavePulses * (double)masterFreq / (double)masterPulses;
          slaveFreq = (unsigned int)lround(fsd);
          if (slaveFreq < 1) slaveFreq = 1;
        }
        
        Serial.print(F("  [SEG ")); Serial.print(idx); 
        Serial.print(F("→")); Serial.print(idx + 1);
        Serial.print(F("] BIG: ")); Serial.print(bigNow);
        Serial.print(F("→")); Serial.print(bigTarget);
        Serial.print(F(" | Z: ")); Serial.print(zNow);
        Serial.print(F("→")); Serial.println(zHedef);
        
        if (masterPulses > 0 && masterFreq > 0) {
          useMotor(MOTOR_B);
          pulseAt(masterPulses, masterYon, masterFreq);
        }
        
        if (slavePulses > 0 && slaveFreq > 0) {
          useMotor(MOTOR_Z);
          pulseAt(slavePulses, slaveYon, slaveFreq);
        }
        
        idx++;
      }
      
      if (pulseAtAktifMi(MOTOR_B)) {
        useMotor(MOTOR_B);
        pulseAt(0, 0, 0);
      }
      
      if (pulseAtAktifMi(MOTOR_Z)) {
        useMotor(MOTOR_Z);
        pulseAt(0, 0, 0);
      }
    }
    break;
    
    default:
      break;
  }
}

// ═══════════════════════════════════════════════════════════════
// DUR/DEVAM FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

void oynatmaDuraklat() {
  if (durum != OY_SEGMENT_OYNAT) {
    Serial.println(F("✗ Oynatma aktif değil!"));
    return;
  }
  
  pulseAtDurdur(MOTOR_B);
  pulseAtDurdur(MOTOR_Z);
  digitalWrite(KAYNAK_ROLE_PIN, HIGH);
  
  durum = OY_DURAKLATILDI;
  
  Serial.println(F("⏸ Oynatma duraklatıldı!"));
  Serial.print(F("  Segment: ")); Serial.println(idx);
}

void oynatmaDevamEt() {
  if (durum != OY_DURAKLATILDI) {
    Serial.println(F("✗ Oynatma duraklatılmamış!"));
    return;
  }
  
  durum = OY_SEGMENT_OYNAT;
  digitalWrite(KAYNAK_ROLE_PIN, LOW);
  
  Serial.println(F("▶ Oynatma devam ediyor!"));
  Serial.print(F("  Segment: ")); Serial.println(idx);
}

bool oynatmaDuraklatildiMi() {
  return (durum == OY_DURAKLATILDI);
}

// ═══════════════════════════════════════════════════════════════
// DURUM FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

bool oynatmaAktifMi() {
  return (durum != OY_KAPALI && durum != OY_TAMAMLANDI && durum != OY_DURAKLATILDI);
}

bool oynatmaTamamlandiMi() {
  return (durum == OY_TAMAMLANDI);
}

uint16_t oynatmaSegmentIndex() {
  return idx;
}

void oynatmaDurdur() {
  Serial.println(F("[OYNATMA] Durduruldu!"));
  
  pulseAtDurdur(MOTOR_B);
  pulseAtDurdur(MOTOR_Z);
  digitalWrite(KAYNAK_ROLE_PIN, HIGH);
  
  durum = OY_KAPALI;
  kayitPtr = nullptr;
}