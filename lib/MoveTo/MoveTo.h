// MoveTo.h - İVMELİ HAREKET MODÜLÜ v4 - TEMİZ VERSİYON
#ifndef MOVETO_H
#define MOVETO_H

#include <Arduino.h>
#include "stepmotorenkoderiokuma.h"
#include "Config.h"

// ═══════════════════════════════════════════════════════════════
// FONKSİYON TANIMLARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Encoder pointer'larını kaydet (setup'ta bir kez çağır)
 */
void moveToSetup(StepMotorEncoder* encZ, 
                 StepMotorEncoder* encX, 
                 StepMotorEncoder* encB);

/**
 * @brief İvmeli hareket başlat
 * 
 * ÇALIŞMA PRENSİBİ:
 * - Motor bazlı rampa parametreleri (Config.h)
 * - 3 faz: Hızlanma → Sabit Hız → Yavaşlama
 * - Salınım modu: Daha kısa rampa (hızlı geçişler)
 * 
 * @param motorIndex Motor (MOTOR_Z=0, MOTOR_X=1, MOTOR_B=2)
 * @param hedefEnc Hedef encoder pozisyonu
 * @param maxHz Maximum hız (Hz)
 * @param salinimModu true = Salınım modu (hızlı rampa)
 *                    false = Normal mod (default)
 * @return true Başarıyla başlatıldı
 * 
 * ÖRNEKLER:
 *   moveTo(MOTOR_X, 10000, 5000);        // Normal: 6000p rampa
 *   moveTo(MOTOR_X, 10000, 8000, true);  // Salınım: 600p rampa
 */
bool moveTo(uint8_t motorIndex, long hedefEnc, unsigned int maxHz, bool salinimModu = false);

/**
 * @brief Motor hareket ediyor mu?
 * @param motorIndex Motor
 * @return true Motor aktif
 */
bool moveToAktifMi(uint8_t motorIndex);

/**
 * @brief MoveTo arka plan döngüsü (her loop'ta çağır!)
 */
void moveToRun();

/**
 * @brief Motor hedefe ulaştı mı? (Edge detection)
 * @param motorIndex Motor
 * @return true Bir kez true döner, sonra false
 */
bool moveToBittiMi(uint8_t motorIndex);

/**
 * @brief Motoru durdur
 * @param motorIndex Motor
 */
void moveToDurdur(uint8_t motorIndex);

/**
 * @brief Tüm motorları durdur
 */
void moveToHepsiniDurdur();

#endif // MOVETO_H