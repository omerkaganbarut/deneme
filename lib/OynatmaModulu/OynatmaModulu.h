// OynatmaModulu.h - v10.0 PUNTA MODU + DİNAMİK A0
#ifndef OYNATMAMODULU_H
#define OYNATMAMODULU_H

#include <Arduino.h>
#include "stepmotorenkoderiokuma.h"
#include "CiftKayitModulu.h"

// ═══════════════════════════════════════════════════════════════
// TEMEL FONKSİYONLAR
// ═══════════════════════════════════════════════════════════════
void oynatmaEncoderSetup(StepMotorEncoder* bigEncoder, StepMotorEncoder* zEncoder);

void oynatmaBaslatKayit(const Sample* kayit, uint16_t ornekSayisi);
void oynatmaRun();

bool oynatmaAktifMi();
bool oynatmaTamamlandiMi();
uint16_t oynatmaSegmentIndex();

void oynatmaDurdur();

// ═══════════════════════════════════════════════════════════════
// DUR/DEVAM FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Oynatmayı duraklat (pause)
 * Motorlar durur, index korunur, kaldığı yerden devam edilebilir
 */
void oynatmaDuraklat();

/**
 * @brief Oynatmaya devam et (resume)
 * Duraklatılan yerden devam eder
 */
void oynatmaDevamEt();

/**
 * @brief Oynatma duraklatıldı mı?
 */
bool oynatmaDuraklatildiMi();

// ═══════════════════════════════════════════════════════════════
// PUNTA MODU FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Punta modunu aç/kapat
 * @param aktif true = Punta modu (5 segment → 2sn kaynak)
 *              false = Normal mod (sürekli kaynak)
 */
void oynatmaPuntaModuAyarla(bool aktif);

/**
 * @brief Punta modu aktif mi?
 */
bool oynatmaPuntaModuAktifMi();

// ═══════════════════════════════════════════════════════════════
// 🔧 PUBLIC HELPER FONKSİYONLAR (Mapping)
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Global A0 aralığına göre Z max offset hesapla
 * @return Z encoder max offset değeri
 */
long oynatmaHesaplaZMaxOffset();

/**
 * @brief A0 → Z encoder offseti mapping
 * @param a0 A0 sensör değeri
 * @return Z encoder offset (0 ile zMaxOffset arası)
 */
long oynatmaMapA0ToZOffset(uint16_t a0);

/**
 * @brief A0 → Z encoder hedef pozisyon (referans + offset)
 * @param a0 A0 sensör değeri
 * @return Z encoder hedef pozisyonu (ckMeta.zRefPos + offset)
 */
long oynatmaMapA0ToZEnc(uint16_t a0);

/**
 * @brief A0 → BIG motor frekans mapping (ters orantılı)
 * @param a0 A0 sensör değeri
 * @return BIG motor frekansı (Hz)
 */
unsigned int oynatmaMapA0ToBigFreq(uint16_t a0);

#endif // OYNATMAMODULU_H