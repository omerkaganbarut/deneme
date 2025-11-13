// MoveSalinim.h - X EKSENİ SALINİM MODÜLÜ v2 - TEMİZ VERSİYON
#ifndef MOVESALINIM_H
#define MOVESALINIM_H

#include <Arduino.h>
#include "stepmotorenkoderiokuma.h"

/**
 * @brief Encoder pointer'ını ayarla (setup'ta bir kez çağır)
 */
void msEncoderSetup(StepMotorEncoder* xEncoder);

/**
 * @brief Salınım başlat (Offset tabanlı)
 * 
 * ÇALIŞMA MANTIĞI:
 * - Şu anki X pozisyonunu merkez olarak kaydet
 * - Merkez + offset ↔ Merkez - offset döngüsü
 * - MoveTo modülünü kullanır (ivmeli hareket)
 * 
 * @param offset Salınım genliği (encoder birimi)
 * @param hiz X motor hızı (Hz)
 * @return true Başarıyla başlatıldı
 * 
 * ÖRNEK:
 *   msBaslat(2000, 8000);  // ±2000 encoder, 8000Hz
 */
bool msBaslat(long offset, unsigned int hiz);

/**
 * @brief Salınım arka plan döngüsü (her loop'ta çağır!)
 */
void msRun();

/**
 * @brief Salınım aktif mi?
 */
bool msAktifMi();

/**
 * @brief Salınımı durdur
 */
void msDurdur();

#endif // MOVESALINIM_H