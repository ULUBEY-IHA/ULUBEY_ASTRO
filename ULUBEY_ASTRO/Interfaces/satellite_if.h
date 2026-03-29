#ifndef SATELLITE_IF_H
#define SATELLITE_IF_H

#include <stdint.h>

/**
 * @brief ADCS stabilizasyon durumlari.
 */
typedef enum
{
  ADCS_NOT_STABLE = 0,
  ADCS_STABLE = 1
} ADCS_Status_t;

/**
 * @brief ADCS durumunu okur.
 * @return ADCS anlik durum bilgisi.
 */
ADCS_Status_t IF_ADCS_GetStatus(void);

/**
 * @brief ADCS calisma modunu ayarlar.
 * @param mode 0: Idle/Sleep, 1: Targeting.
 */
void IF_ADCS_SetMode(uint8_t mode);

/**
 * @brief GPS konum bilgisini alir.
 * @param[out] lat Enlem cikisi.
 * @param[out] lon Boylam cikisi.
 */
void IF_GPS_GetLocation(float *lat, float *lon);

/**
 * @brief Kamera cekimini tetikler.
 */
void IF_Camera_Capture(void);

/**
 * @brief Radyo uzerinden veri gonderimini tetikler.
 */
void IF_Radio_Transmit(void);

#endif /* SATELLITE_IF_H */
