#ifndef SATELLITE_IF_H
#define SATELLITE_IF_H

#include <stdint.h>

typedef enum
{
  ADCS_NOT_STABLE = 0,
  ADCS_STABLE = 1
} ADCS_Status_t;

ADCS_Status_t IF_ADCS_GetStatus(void);
void IF_ADCS_SetMode(uint8_t mode);
void IF_GPS_GetLocation(float *lat, float *lon);
void IF_Camera_Capture(void);
void IF_Radio_Transmit(void);

#endif /* SATELLITE_IF_H */
