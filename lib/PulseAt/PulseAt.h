// PulseAt.h - STEP MOTOR PARALEL SÜRÜŞ MODÜLÜ v3
#ifndef PULSEAT_H
#define PULSEAT_H

#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
// TEMEL FONKSİYONLAR
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Aktif motoru seç
 * @param motorIndex Motor (MOTOR_Z=0, MOTOR_X=1, MOTOR_B=2)
 */
void useMotor(uint8_t motorIndex);

/**
 * @brief Pulse gönder (non-blocking)
 * 
 * İLK ÇAĞRI (iş başlatma):
 *   useMotor(MOTOR_X);
 *   pulseAt(1000, 0, 5000);  // 1000 pulse, ileri, 5000Hz
 * 
 * ARKA PLAN (her loop'ta):
 *   useMotor(MOTOR_X);
 *   pulseAt(0, 0, 0);  // Mevcut işi sürdür
 * 
 * @param toplamPulse Kaç pulse gönderilecek (0 = arka plan çalışması)
 * @param yon Yön (0=ileri, 1=geri)
 * @param hertz Frekans (Hz)
 */
void pulseAt(unsigned long toplamPulse, int yon, unsigned int hertz);

/**
 * @brief Motor aktif mi?
 * @param motorIndex Motor
 * @return true Motor pulse gönderiyor
 */
bool pulseAtAktifMi(uint8_t motorIndex);

/**
 * @brief Motor az önce bitti mi? (Edge detection)
 * @param motorIndex Motor
 * @return true Bir kez true döner, sonra false
 */
bool pulseAtBittiMi(uint8_t motorIndex);

// ═══════════════════════════════════════════════════════════════
// DURDURMA FONKSİYONLARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief SEGMENT GEÇİŞİ DURDURMA (ENA LOW kalsın)
 * 
 * KULLANIM ALANLARI:
 * - Segment geçişlerinde (Kayıt/Oynatma)
 * - Başlangıç konumunda beklerken (Y/N onayı)
 * - Motor pozisyonunu korumak istediğinde
 * 
 * NE YAPAR:
 * - Sadece pulse gönderimini durdurur
 * - Motor fiziksel olarak AKTİF kalır (ENA LOW)
 * - Motor pozisyonunu tutar
 * 
 * @param motorIndex Motor
 */
void pulseAtDurdur(uint8_t motorIndex);

/**
 * @brief İŞLEM TAMAMLAMA DURDURMA (ENA HIGH - enerji kes)
 * 
 * KULLANIM ALANLARI:
 * - Kayıt/Oynatma tamamen bittiğinde
 * - MoveTo tamamlandığında (uzun süreli)
 * - Kullanıcı başka işlerle uğraşırken
 * 
 * NE YAPAR:
 * - Pulse gönderimini durdurur
 * - Motor fiziksel olarak DEVRE DIŞI (ENA HIGH)
 * - Enerji tasarrufu sağlar
 * 
 * @param motorIndex Motor
 */
void pulseAtTamamla(uint8_t motorIndex);

/**
 * @brief ACİL DURDURMA (ENA HIGH - hemen kes!)
 * 
 * KULLANIM ALANLARI:
 * - Acil durdurma butonu ('S' komutu)
 * - Hata durumlarında
 * - Kullanıcı müdahalesi gerektiğinde
 * 
 * NE YAPAR:
 * - Pulse gönderimini durdurur
 * - Motor fiziksel olarak HEMEN DURDURULUR (ENA HIGH)
 * 
 * @param motorIndex Motor
 */
void pulseAtAcilDurdur(uint8_t motorIndex);

/**
 * @brief Tüm motorları ACİL DURDUR
 */
void pulseAtHepsiniDurdur();

/**
 * @brief Tüm motorları TAMAMLA (enerji kes)
 */
void pulseAtHepsiniTamamla();

#endif // PULSEAT_H