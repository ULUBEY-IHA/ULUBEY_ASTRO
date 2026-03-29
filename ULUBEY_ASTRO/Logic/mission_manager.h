#ifndef MISSION_MANAGER_H
#define MISSION_MANAGER_H

/**
 * @brief Uydu gorev akisini yuruten ana FreeRTOS task fonksiyonu.
 * @param argument RTOS tarafindan gecilen task argumani.
 */
void vTaskMissionManager(void *argument);

#endif /* MISSION_MANAGER_H */
