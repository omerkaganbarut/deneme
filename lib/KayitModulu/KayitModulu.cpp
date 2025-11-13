// KayitModulu.cpp - TEMİZ VERSİYON v4
#include "KayitModulu.h"
#include "Config.h"
#include "PulseAt.h"
#include "stepmotorenkoderiokuma.h"
#include "A0Filtre.h"

// ═══════════════════════════════════════════════════════════════
// DURUM MAKİNESİ
// ═══════════════════════════════════════════════════════════════
enum KayitDurum {
  KAYIT_KAPALI = 0,
  KAYIT_BASLANGIC,
  KAYIT_BASLANGIC_BEKLE,
  KAYIT_MOTOR_CALISMA,
  KAYIT_MOTOR_BEKLE,
  KAYIT_ORNEK_AL,
  KAYIT_TAMAMLANDI
};

// ═══════════════════════════════════════════════════════════════
// STATİK DEĞİŞKENLER
// ═══════════════════════════════════════════════════════════════
static Sample samples[KAYIT_ORNEK_SAYISI];
static uint16_t idx = 0;
static KayitDurum durum = KAYIT_KAPALI;
static unsigned long beklemeBaslangic = 0;
static StepMotorEncoder* bigEnc = nullptr;
static long baslangicEnc = 0;
static int kayitYon = 0;

// ═══════════════════════════════════════════════════════════════
// ENCODER SETUP
// ═══════════════════════════════════════════════════════════════
void kayitEncoderSetup(StepMotorEncoder* bigEncoder) {
  bigEnc = bigEncoder;
}

// ═══════════════════════════════════════════════════════════════
// KAYIT BAŞLATMA
// ═══════════════════════════════════════════════════════════════
void kayitBaslat(int yon) {
  Serial.println(F("\n[KAYIT] Başlatılıyor..."));
  
  idx = 0;
  durum = KAYIT_BASLANGIC;
  
  // Motor enable
  pinMode(ENA1_PIN, OUTPUT);
  pinMode(ENA2_PIN, OUTPUT);
  pinMode(ENA3_PIN, OUTPUT);
  digitalWrite(ENA1_PIN, LOW);
  digitalWrite(ENA2_PIN, LOW);
  digitalWrite(ENA3_PIN, LOW);
  delay(2000);
  
  if (bigEnc == nullptr) {
    Serial.println(F("✗ Encoder hatası!"));
    return;
  }
  
  baslangicEnc = bigEnc->getPosition();
  
  // İlk örneği al
  samples[0].enc = baslangicEnc;
  samples[0].a0 = a0FiltreliOku();
  
  Serial.print(F("  Yön: "));
  Serial.println(yon ? F("Geri (16000→0)") : F("İleri (0→16000)"));
  Serial.print(F("  Başlangıç Enc: "));
  Serial.print(baslangicEnc);
  Serial.print(F(" | İlk A0: "));
  Serial.println(samples[0].a0);
  
  kayitYon = yon;
  
  beklemeBaslangic = millis();
  durum = KAYIT_BASLANGIC_BEKLE;
  
  Serial.println(F("✓ Başlatıldı\n"));
}

// ═══════════════════════════════════════════════════════════════
// KAYIT ARKA PLAN
// ═══════════════════════════════════════════════════════════════
void kayitRun() {
  switch (durum) {
    
    case KAYIT_KAPALI:
      return;
    
    case KAYIT_BASLANGIC_BEKLE:
      // 1 saniye bekle
      if (millis() - beklemeBaslangic >= 1000) {
        durum = KAYIT_MOTOR_CALISMA;
        idx = 1;
        
        Serial.print(F("[SEG 0→1]"));
        
        // İlk segment başlat
        useMotor(MOTOR_B);
        pulseAt(KAYIT_ARALIK, kayitYon, KAYIT_HZ);
        
        durum = KAYIT_MOTOR_BEKLE;
      }
      break;
    
    case KAYIT_MOTOR_BEKLE:
      // pulseAt arka planını çalıştır
      useMotor(MOTOR_B);
      pulseAt(0, 0, 0);
      
      // Segment bitti mi?
      if (pulseAtBittiMi(MOTOR_B)) {
        // Hedef kontrolü
        long mevcutEnc = bigEnc->getPosition();
        long teoriHedef;
        
        if (kayitYon == 0) {
          teoriHedef = baslangicEnc + (KAYIT_ARALIK * idx);
        } else {
          teoriHedef = baslangicEnc - (KAYIT_ARALIK * idx);
        }
        
        long fark = teoriHedef - mevcutEnc;
        
        // Sapma raporu
        if (abs(fark) >= 2) {
          Serial.print(F("   ⚠ Sapma: "));
          Serial.print(fark);
          Serial.print(F(" pulse (Hedef: "));
          Serial.print(teoriHedef);
          Serial.print(F(", Mevcut: "));
          Serial.print(mevcutEnc);
          Serial.println(F(")"));
        }
        
        durum = KAYIT_ORNEK_AL;
      }
      break;
    
    case KAYIT_ORNEK_AL:
      // Örnek al
      samples[idx].enc = bigEnc->getPosition();
      samples[idx].a0 = a0FiltreliOku();
      
      Serial.print(F(" → ["));
      Serial.print(idx);
      Serial.print(F("] enc="));
      Serial.print(samples[idx].enc);
      Serial.print(F(", a0="));
      Serial.println(samples[idx].a0);
      
      // Son örnek mi?
      if (idx >= KAYIT_ORNEK_SAYISI - 1) {
        durum = KAYIT_TAMAMLANDI;
        Serial.println(F("\n[KAYIT] Tamamlandı! ✓\n"));
      } else {
        idx++;
        
        long mevcutEnc = bigEnc->getPosition();
        long sonrakiTeoriHedef;
        
        if (kayitYon == 0) {
          sonrakiTeoriHedef = baslangicEnc + (KAYIT_ARALIK * idx);
        } else {
          sonrakiTeoriHedef = baslangicEnc - (KAYIT_ARALIK * idx);
        }
        
        long gerekliFark = sonrakiTeoriHedef - mevcutEnc;
        unsigned long pulseSayisi = abs(gerekliFark);
        int yon = kayitYon;
        
        // Segment başlat
        useMotor(MOTOR_B);
        pulseAt(pulseSayisi, yon, (unsigned int)KAYIT_HZ);
        
        durum = KAYIT_MOTOR_BEKLE;
      }
      break;
    
    case KAYIT_TAMAMLANDI:
      return;
    
    default:
      Serial.println(F("✗ Bilinmeyen durum!"));
      durum = KAYIT_KAPALI;
      break;
  }
}

// ═══════════════════════════════════════════════════════════════
// DURUM FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

bool kayitTamamlandiMi() {
  return (durum == KAYIT_TAMAMLANDI);
}

uint16_t kayitOrnekSayisi() {
  if (durum == KAYIT_TAMAMLANDI) {
    return KAYIT_ORNEK_SAYISI;
  }
  return idx;
}

const Sample* kayitVerileri() {
  return samples;
}

// ═══════════════════════════════════════════════════════════════
// DURDURMA
// ═══════════════════════════════════════════════════════════════
void kayitDurdur() {
  Serial.println(F("\n[KAYIT] Durduruldu!"));
  
  pulseAtDurdur(MOTOR_B);
  
  durum = KAYIT_KAPALI;
  
  Serial.print(F("   Toplanan örnek: "));
  Serial.println(idx);
  Serial.println();
}