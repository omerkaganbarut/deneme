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

// ═══════════════════════════════════════════════════════════════
// DATA EXCHANGE FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════
void ckExport1();  // W1 formatında Kayıt1'i export et
void ckExport2();  // W2 formatında Kayıt2'yi export et
void ckExport3();  // W3 formatında Meta data'yı export et

bool ckImport1(String veriStr);  // W1 formatında Kayıt1'i import et
bool ckImport2(String veriStr);  // W2 formatında Kayıt2'yi import et
bool ckImport3(String veriStr);  // W3 formatında Meta data'yı import et

void ckTemizle1();
void ckTemizle2();
void ckHepsiniTemizle();
// CiftKayitModulu.h - STREAM FONKSİYONLARI EKLENDI
// ... mevcut kodlar aynı kalacak ...

// ═══════════════════════════════════════════════════════════════
// STREAM IMPORT/EXPORT FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Kayıt1'i stream modunda import et
 * Format: WS1 → sonra veri akışı başlar
 * Örnek: 0,512 100,523 200,534 ... END
 */
bool ckImportStream1();

/**
 * @brief Kayıt2'yi stream modunda import et
 * Format: WS2 → sonra veri akışı başlar
 */
bool ckImportStream2();

/**
 * @brief Kayıt1'i stream modunda export et
 * 10 örnek sonra nokta koyar, devam eder
 */
void ckExportStream1();

/**
 * @brief Kayıt2'yi stream modunda export et
 */
void ckExportStream2();

// Global A0 aralığı (backward compatibility için)
extern uint16_t globalA0Min;
extern uint16_t globalA0Max;

#endif // CIFTKAYITMODULU_H