// MoveSalinim.cpp - X EKSENİ SALINİM MODÜLÜ v2 - TEMİZ VERSİYON
#include "MoveSalinim.h"
#include "MoveTo.h"
#include "Config.h"

// ═══════════════════════════════════════════════════════════════
// SALINİM DURUMU
// ═══════════════════════════════════════════════════════════════
struct MSState {
  bool aktif = false;
  
  long merkez = 0;
  long offset = 0;
  unsigned int hiz = 0;
  
  bool sagaTaraf = true;
  bool hareketBasladi = false;
};

static MSState MS;

// ═══════════════════════════════════════════════════════════════
// ENCODER POINTER
// ═══════════════════════════════════════════════════════════════
static StepMotorEncoder* gXEnc = nullptr;

// ═══════════════════════════════════════════════════════════════
// ENCODER SETUP
// ═══════════════════════════════════════════════════════════════
void msEncoderSetup(StepMotorEncoder* xEncoder) {
  gXEnc = xEncoder;
}

// ═══════════════════════════════════════════════════════════════
// SALINİM BAŞLAT
// ═══════════════════════════════════════════════════════════════
bool msBaslat(long offset, unsigned int hiz) {
  if (!gXEnc) {
    Serial.println(F("MS: Encoder setup yapilmamis!"));
    return false;
  }
  
  if (offset == 0) {
    Serial.println(F("MS: Offset 0 olamaz!"));
    return false;
  }
  
  if (hiz == 0) {
    Serial.println(F("MS: Hiz 0 olamaz!"));
    return false;
  }
  
  MS.merkez = gXEnc->getPosition();
  MS.offset = abs(offset);
  MS.hiz = hiz;
  MS.sagaTaraf = true;
  MS.hareketBasladi = false;
  MS.aktif = true;
  
  return true;
}

// ═══════════════════════════════════════════════════════════════
// SALINİM ARKA PLAN DÖNGÜSÜ
// ═══════════════════════════════════════════════════════════════
void msRun() {
  if (!MS.aktif) return;
  
  if (!MS.hareketBasladi) {
    long hedef;
    
    if (MS.sagaTaraf) {
      hedef = MS.merkez + MS.offset;
    } else {
      hedef = MS.merkez - MS.offset;
    }
    
    moveTo(MOTOR_X, hedef, MS.hiz, true);
    MS.hareketBasladi = true;
    return;
  }
  
  if (moveToBittiMi(MOTOR_X)) {
    MS.sagaTaraf = !MS.sagaTaraf;
    MS.hareketBasladi = false;
  }
}

// ═══════════════════════════════════════════════════════════════
// AKTİF Mİ?
// ═══════════════════════════════════════════════════════════════
bool msAktifMi() {
  return MS.aktif;
}

// ═══════════════════════════════════════════════════════════════
// DURDUR
// ═══════════════════════════════════════════════════════════════
void msDurdur() {
  if (!MS.aktif) return;
  
  moveToDurdur(MOTOR_X);
  
  MS.aktif = false;
  MS.hareketBasladi = false;
  
  Serial.println(F("MS: Salinim durduruldu!"));
}