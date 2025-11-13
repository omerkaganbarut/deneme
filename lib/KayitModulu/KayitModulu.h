// KayitModulu.h - TEMİZ VERSİYON
#ifndef KAYITMODULU_H
#define KAYITMODULU_H

#include <Arduino.h>
#include "stepmotorenkoderiokuma.h"
#include "Config.h"  // Sample struct burada tanımlı

// ═══════════════════════════════════════════════════════════════
// FONKSİYON TANIMLARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Encoder pointer'ını ayarla (setup'ta bir kez çağır)
 * @param bigEncoder BIG motor encoder'ı
 */
void kayitEncoderSetup(StepMotorEncoder* bigEncoder);

/**
 * @brief Kayıt başlat
 * @param yon Kayıt yönü (0=ileri: 0→16000, 1=geri: 16000→0)
 */
void kayitBaslat(int yon);

/**
 * @brief Kayıt arka plan döngüsü (her loop'ta çağır!)
 */
void kayitRun();

/**
 * @brief Kayıt tamamlandı mı?
 * @return true Kayıt tamamlandı
 */
bool kayitTamamlandiMi();

/**
 * @brief Toplanan örnek sayısı
 * @return Örnek sayısı (tamamlanmadıysa mevcut index)
 */
uint16_t kayitOrnekSayisi();

/**
 * @brief Kayıt verilerine read-only erişim
 * @return Sample array pointer'ı
 */
const Sample* kayitVerileri();

/**
 * @brief Kayıt işlemini durdur (acil durdurma)
 */
void kayitDurdur();

#endif // KAYITMODULU_H