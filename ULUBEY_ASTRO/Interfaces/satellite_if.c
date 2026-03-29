#include "satellite_if.h"

#include <stdio.h>

ADCS_Status_t IF_ADCS_GetStatus(void)
{
  printf("[IF][ADCS] status=STABLE\r\n");
  return ADCS_STABLE;
}

void IF_ADCS_SetMode(uint8_t mode)
{
  printf("[IF][ADCS] mode=%u\r\n", (unsigned int)mode);
}

void IF_GPS_GetLocation(float *lat, float *lon)
{
  if ((lat != 0) && (lon != 0))
  {
    *lat = 40.5f;
    *lon = 29.0f;
  }

  printf("[IF][GPS] lat=40.5000 lon=29.0000\r\n");
}

void IF_Camera_Capture(void)
{
  printf("[IF][CAM] capture=TRIGGERED\r\n");
}

void IF_Radio_Transmit(void)
{
  printf("[IF][RADIO] tx=TRIGGERED\r\n");
}
