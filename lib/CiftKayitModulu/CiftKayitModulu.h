// CiftKayitModulu.h - v8.0 AYRILMIŞ META DATA
#ifndef CIFTKAYITMODULU_H
#define CIFTKAYITMODULU_H

#include <Arduino.h>
#include "stepmotorenkoderiokuma.h"
#include "Config.h"

// ═══════════════════════════════════════════════════════════════
// KAYIT META VERİLERİ
// ═══════════════════════════════════════════════════════════════
struct CK_MetaData {
  long x1Pos;           // Kayıt1 X pozisyonu
  long x2Pos;           // Kayıt2 X pozisyonu
  long zRefPos;         // Z referans pozisyonu
  uint16_t globalA0Min; // İki kayıttan da en küçük A0
  uint16_t globalA0Max; // İki kayıttan da en büyük A0
  bool valid;           // Veri geçerli mi?
};

// ═══════════════════════════════════════════════════════════════
// EXTERN KAYIT ARRAYLERI VE META VERİLER
// ═══════════════════════════════════════════════════════════════
extern Sample kayit1[KAYIT_ORNEK_SAYISI];
extern Sample kayit2[KAYIT_ORNEK_SAYISI];
extern CK_MetaData ckMeta;

// ═══════════════════════════════════════════════════════════════
// TEMEL FONKSİYONLAR
// ═══════════════════════════════════════════════════════════════
void ckEncoderSetup(StepMotorEncoder* bigEncoder, StepMotorEncoder* xEncoder);
void ckBaslat(long x1Enc, long x2Enc, int kayit1Yon, int kayit2Yon);
void ckRun();
bool ckAktifMi();
bool ckTamamlandiMi();
void ckDurdur();
void ckExport3();  // W3 formatında Meta data'yı export et
bool ckImport3(String veriStr);  // W3 formatında Meta data'yı import et
void ckTemizle1();
void ckTemizle2();
void ckHepsiniTemizle();
bool ckImportStream1();
bool ckImportStream2();
void ckExportStream1();
void ckExportStream2();

// Global A0 aralığı (backward compatibility için)
extern uint16_t globalA0Min;
extern uint16_t globalA0Max;

#endif // CIFTKAYITMODULU_H