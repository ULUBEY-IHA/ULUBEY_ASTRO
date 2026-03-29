#include "satellite_if.h"

#include <stdio.h>

/* Placeholder implementation: ADCS her sorguda stabil doner. */
ADCS_Status_t IF_ADCS_GetStatus(void)
{
  printf("[IF][ADCS] status=STABLE\r\n");
  return ADCS_STABLE;
}

/* Placeholder implementation: sadece secilen modu loglar. */
void IF_ADCS_SetMode(uint8_t mode)
{
  printf("[IF][ADCS] mode=%u\r\n", (unsigned int)mode);
}

/* Test kolayligi icin sabit Bursa koordinatlari donulur. */
void IF_GPS_GetLocation(float *lat, float *lon)
{
  if ((lat != 0) && (lon != 0))
  {
    *lat = 40.5f;
    *lon = 29.0f;
  }

  printf("[IF][GPS] lat=40.5000 lon=29.0000\r\n");
}

/* Placeholder implementation: kamera tetigini loglar. */
void IF_Camera_Capture(void)
{
  printf("[IF][CAM] capture=TRIGGERED\r\n");
}

/* Placeholder implementation: radyo gonderimini loglar. */
void IF_Radio_Transmit(void)
{
  printf("[IF][RADIO] tx=TRIGGERED\r\n");
}
