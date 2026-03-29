#include "mission_manager.h"

#include "../Interfaces/satellite_if.h"

#include "cmsis_os.h"
#include <stdio.h>

#define TARGET_MIN_LAT (40.0f)
#define TARGET_MAX_LAT (41.0f)
#define TARGETING_MODE  (1U)
#define IDLE_MODE       (0U)

void vTaskMissionManager(void *argument)
{
  float lat = 0.0f;
  float lon = 0.0f;
  ADCS_Status_t adcsStatus = ADCS_NOT_STABLE;
  uint32_t stableRetryCount = 0U;

  (void)argument;

  for (;;)
  {
    IF_GPS_GetLocation(&lat, &lon);

    if ((lat >= TARGET_MIN_LAT) && (lat <= TARGET_MAX_LAT))
    {
      printf("[LOGIC][MISSION] target-zone ENTER lat=%.4f lon=%.4f\r\n", lat, lon);
      IF_ADCS_SetMode(TARGETING_MODE);

      stableRetryCount = 0U;
      adcsStatus = ADCS_NOT_STABLE;

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
        printf("[ERROR]: ADCS Timeout!\r\n");
        IF_ADCS_SetMode(IDLE_MODE);
        continue;
      }

      printf("[LOGIC][MISSION] adcs-stable payload-sequence-start\r\n");
      IF_Camera_Capture();
      IF_Radio_Transmit();
    }
    else
    {
      printf("[LOGIC][MISSION] outside-target-zone lat=%.4f lon=%.4f\r\n", lat, lon);
      IF_ADCS_SetMode(IDLE_MODE);
    }

    osDelay(1000U);
  }
}
