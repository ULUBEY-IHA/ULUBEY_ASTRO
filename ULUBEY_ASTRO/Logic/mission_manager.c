#include "mission_manager.h"

#include "../Interfaces/satellite_if.h"

#include "cmsis_os.h"
#include <stdio.h>

#define TARGET_MIN_LAT (40.0f)
#define TARGET_MAX_LAT (41.0f)
#define TARGETING_MODE  (1U)
#define IDLE_MODE       (0U)

/*
 * Mission akisi:
 * 1) Konum oku
 * 2) Hedef bolgedeyse ADCS'i hedefleme moduna al
 * 3) ADCS stabil olana kadar sinirli sure bekle
 * 4) Stabil olursa payload adimini tetikle
 * 5) Hedef disinda ise ADCS'i idle moda al
 */
void vTaskMissionManager(void *argument)
{
  float lat = 0.0f;
  float lon = 0.0f;
  ADCS_Status_t adcsStatus = ADCS_NOT_STABLE;
  uint32_t stableRetryCount = 0U;

  (void)argument;

  for (;;)
  {
    /* Periyodik konum guncellemesi. */
    IF_GPS_GetLocation(&lat, &lon);

    if ((lat >= TARGET_MIN_LAT) && (lat <= TARGET_MAX_LAT))
    {
      printf("[LOGIC][MISSION] target-zone ENTER lat=%.4f lon=%.4f\r\n", lat, lon);
      IF_ADCS_SetMode(TARGETING_MODE);

      stableRetryCount = 0U;
      adcsStatus = ADCS_NOT_STABLE;

      /* 50 x 200ms ~= 10s timeout ile stabilizasyon polling'i. */
      while (stableRetryCount < 50U)
      {
        adcsStatus = IF_ADCS_GetStatus();

        if (adcsStatus == ADCS_STABLE)
        {
          break;
        }

        printf("[LOGIC][MISSION] waiting-adcs-stable try=%lu/50\r\n", (unsigned long)(stableRetryCount + 1U));
        stableRetryCount++;
        osDelay(200U);
      }

      if (adcsStatus != ADCS_STABLE)
      {
        /* Timeout durumunda fail-safe olarak idle moda donulur. */
        printf("[ERROR]: ADCS Timeout!\r\n");
        IF_ADCS_SetMode(IDLE_MODE);
        continue;
      }

      /* Stabil olduktan sonra payload islemleri tetiklenir. */
      printf("[LOGIC][MISSION] adcs-stable payload-sequence-start\r\n");
      IF_Camera_Capture();
      IF_Radio_Transmit();
    }
    else
    {
      printf("[LOGIC][MISSION] outside-target-zone lat=%.4f lon=%.4f\r\n", lat, lon);
      IF_ADCS_SetMode(IDLE_MODE);
    }

    /* Mission loop periodu: 1 saniye. */
    osDelay(1000U);
  }
}
