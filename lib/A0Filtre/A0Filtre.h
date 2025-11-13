// A0Filtre.h
#ifndef A0FILTRE_H
#define A0FILTRE_H

#include <Arduino.h>

/**
 * @brief A0 sensöründen filtrelenmiş değer oku
 * 
 * ÇALIŞMA PRENSİBİ:
 * - N örnek al (Config.h: A0_FILTER_SAMPLES)
 * - Mod (en çok tekrar eden değer) hesapla
 * - Gürültü filtrele
 * 
 * NOT: pinMode(OPKON_PIN, INPUT) setup()'ta yapılmalı!
 * 
 * @return Filtrelenmiş A0 değeri (0-1023)
 */
uint16_t a0FiltreliOku();

#endif // A0FILTRE_H