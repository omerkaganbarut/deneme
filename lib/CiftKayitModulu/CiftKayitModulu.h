// CiftKayitModulu.h - v9.0 TEMİZ VERSİYON
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
  float bigFreqRef;     // BIG motor referans hızı (Hz) - depo kenarında
  float depoCapMm;      // Depo çapı (mm) - kısa kenar
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
// EXPORT/IMPORT FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════
void ckExport3();  // W3 formatında Meta data'yı export et
bool ckImport3(String veriStr);  // W3 formatında Meta data'yı import et
void ckExportStream1();  // Kayıt1 stream export
void ckExportStream2();  // Kayıt2 stream export
bool ckImportStream1();  // Kayıt1 stream import
bool ckImportStream2();  // Kayıt2 stream import

// ═══════════════════════════════════════════════════════════════
// TEMİZLEME FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════
void ckTemizle1();
void ckTemizle2();
void ckHepsiniTemizle();

// ═══════════════════════════════════════════════════════════════
// HESAPLAMA FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Kayıt1 ve Kayıt2'den global A0 min/max hesapla
 * @param outMin Çıkış: En küçük A0 değeri
 * @param outMax Çıkış: En büyük A0 değeri
 */
void ckHesaplaGlobalA0MinMax(uint16_t* outMin, uint16_t* outMax);

#endif // CIFTKAYITMODULU_H