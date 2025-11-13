#ifndef CONFIG_H
#define CONFIG_H

// ═══════════════════════════════════════════════════════════════
// GENEL VERİ YAPILARI
// ═══════════════════════════════════════════════════════════════

/**
 * @brief Kayıt örnek verisi
 * Tüm kayıt modülleri tarafından kullanılır (CK, KM, Oynatma)
 */
struct Sample {
  long enc;      // Encoder pozisyonu
  uint16_t a0;   // A0 sensör değeri (0-1023)
};

// ═══════════════════════════════════════════════════════════════
// STEP MOTOR PİNLERİ
// ═══════════════════════════════════════════════════════════════

// Motor 1 (Z - Torch Yukarı/Aşağı)
#define STEP1_PIN  5
#define DIR1_PIN   36
#define ENA1_PIN   37

// Motor 2 (X - Torch Sağa/Sola)
#define STEP2_PIN  6
#define DIR2_PIN   39
#define ENA2_PIN   38

// Motor 3 (BIG - Depo Dönme)
#define STEP3_PIN  7
#define DIR3_PIN   40
#define ENA3_PIN   41

// ═══════════════════════════════════════════════════════════════
// ENKODER PİNLERİ (Step Motorlar)
// ═══════════════════════════════════════════════════════════════
#define ENC1_A_PIN  3   // Motor X - interrupt destekli
#define ENC1_B_PIN  10

#define ENC2_A_PIN  2   // Motor Z - interrupt destekli
#define ENC2_B_PIN  9

#define ENC3_A_PIN  18  // Motor BIG - interrupt destekli
#define ENC3_B_PIN  11

// ═══════════════════════════════════════════════════════════════
// LİNEER POTANSİYOMETRE (OPKON)
// ═══════════════════════════════════════════════════════════════
#define OPKON_PIN   A0  // Analog giriş

// ═══════════════════════════════════════════════════════════════
// BUTONLAR
// ═══════════════════════════════════════════════════════════════
// Kalıcı (Toggle) Butonlar
#define TOGGLE1_PIN 22
#define TOGGLE2_PIN 23
#define TOGGLE3_PIN 24

// Push (Momentary) Butonlar
#define PUSH1_PIN   25
#define PUSH2_PIN   26
#define PUSH3_PIN   27

// OUTPUTS
#define KAYNAK_ROLE_PIN 14

// Acil Durdurma (kullanılmıyor ama pin tanımlı)
#define EMERGENCY_STOP_PIN 28

// ═══════════════════════════════════════════════════════════════
// KAYIT MODU PARAMETRELERİ
// ═══════════════════════════════════════════════════════════════
#define KAYIT_TOPLAM_PULSE   16000
#define KAYIT_ARALIK         100
#define KAYIT_HZ             100
#define KAYIT_ORNEK_SAYISI   (KAYIT_TOPLAM_PULSE / KAYIT_ARALIK + 1)  // 161

// ═══════════════════════════════════════════════════════════════
// MOTOR İNDEX TANIMLARI
// ═══════════════════════════════════════════════════════════════
#define MOTOR_Z  0  // 1. motor (Z ekseni - Torch)
#define MOTOR_X  1  // 2. motor (X ekseni - Yatay hareket)
#define MOTOR_B  2  // 3. motor (BIG motor - Depo)

// ═══════════════════════════════════════════════════════════════
// PIN DİZİLERİ (Motor index sırasına göre)
// ═══════════════════════════════════════════════════════════════
#define STEP_PINS { STEP1_PIN, STEP2_PIN, STEP3_PIN }
#define DIR_PINS  { DIR1_PIN,  DIR2_PIN,  DIR3_PIN  }
#define ENA_PINS  { ENA1_PIN,  ENA2_PIN,  ENA3_PIN  }

// ═══════════════════════════════════════════════════════════════
// PULSE AYARLARI
// ═══════════════════════════════════════════════════════════════
#define PULSE_HIGH_US  10  // Pulse HIGH süresi (mikrosaniye)

// ═══════════════════════════════════════════════════════════════
// ANALOG FİLTRE PARAMETRELERİ (A0 için)
// ═══════════════════════════════════════════════════════════════
#define A0_FILTER_SAMPLES      150  // Örnek sayısı (mod hesabı için)
#define A0_FILTER_SPACING_US   150  // Örnekler arası bekleme (µs)

// ═══════════════════════════════════════════════════════════════
// MOTOR BAZLI İVME PARAMETRELERİ
// ═══════════════════════════════════════════════════════════════

// BIG Motor (Ağır yük, kısa mesafe)
#define ACCEL_RAMP_BIG   400    // Kısa rampa (hızlı ivmelenme)
#define MIN_SPEED_BIG    100    // Başlangıç hızı (Hz)

// X Motor (Hafif, uzun mesafe)
#define ACCEL_RAMP_X     6000   // Uzun rampa (yumuşak ivmelenme)
#define MIN_SPEED_X      100    // Düşük başlangıç (Hz)

// Z Motor (Torch, hassas)
#define ACCEL_RAMP_Z     6000   // Çok uzun rampa (çok yumuşak)
#define MIN_SPEED_Z      100    // Çok düşük başlangıç (Hz)

// X Motor Salınım (Hızlı geçişler için)
#define ACCEL_RAMP_X_SALINIM  600   // Kısa rampa (hızlı ivme)
#define MIN_SPEED_X_SALINIM   1000  // Orta başlangıç (Hz)

// ═══════════════════════════════════════════════════════════════
// FİZİKSEL SABITLER
// ═══════════════════════════════════════════════════════════════
#define Z_ENCODER_MAX  160000L  // Z motor maksimum encoder (400mm için)
#define A0_ADC_MAX     1023     // ADC çözünürlüğü

// ═══════════════════════════════════════════════════════════════
// A0 FİZİKSEL ÇÖZÜNÜRLÜK
// ═══════════════════════════════════════════════════════════════
// A0 sensörünün fiziksel çözünürlüğü (1023 A0 birimi = kaç mm)
#define A0_FIZIKSEL_ARALIK_MM 400.0

#endif // CONFIG_H