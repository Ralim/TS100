// By Ben V. Brown - V2.0 of the TS100 firmware

/*
 * Main.cpp bootstraps the device and then hands over to FreeRTOS and the threads
 */

#include "BSP.h"
#include "LIS2DH12.hpp"
#include "Settings.h"
#include "cmsis_os.h"
#include <MMA8652FC.hpp>
#include <main.hpp>
#include <power.hpp>
uint8_t DetectedAccelerometerVersion = 0;
bool    settingsWereReset            = false;
// FreeRTOS variables

osThreadId          GUITaskHandle;
static const size_t GUITaskStackSize = 1024 / 4;
uint32_t            GUITaskBuffer[GUITaskStackSize];
osStaticThreadDef_t GUITaskControlBlock;

osThreadId          PIDTaskHandle;
static const size_t PIDTaskStackSize = 512 / 4;
uint32_t            PIDTaskBuffer[PIDTaskStackSize];
osStaticThreadDef_t PIDTaskControlBlock;

osThreadId          MOVTaskHandle;
static const size_t MOVTaskStackSize = 1024 / 4;
uint32_t            MOVTaskBuffer[MOVTaskStackSize];
osStaticThreadDef_t MOVTaskControlBlock;

osThreadId          POWTaskHandle;
static const size_t POWTaskStackSize = 512 / 4;
uint32_t            POWTaskBuffer[POWTaskStackSize];
osStaticThreadDef_t POWTaskControlBlock;

// End FreeRTOS
// Main sets up the hardware then hands over to the FreeRTOS kernel
int main(void) {
  preRToSInit();
  setTipX10Watts(0); // force tip off
  resetWatchdog();
  OLED::setFont(0); // default to bigger font
  // Testing for which accelerometer is mounted
  settingsWereReset = restoreSettings(); // load the settings from flash
  resetWatchdog();
  /* Create the thread(s) */
  /* definition and creation of POWTask - Power management for QC */
  osThreadStaticDef(POWTask, startPOWTask, osPriorityAboveNormal, 0, POWTaskStackSize, POWTaskBuffer, &POWTaskControlBlock);
  POWTaskHandle = osThreadCreate(osThread(POWTask), NULL);

  /* definition and creation of GUITask - The OLED control & update*/
  osThreadStaticDef(GUITask, startGUITask, osPriorityBelowNormal, 0, GUITaskStackSize, GUITaskBuffer, &GUITaskControlBlock);
  GUITaskHandle = osThreadCreate(osThread(GUITask), NULL);

  /* definition and creation of PIDTask - Heating control*/
  osThreadStaticDef(PIDTask, startPIDTask, osPriorityRealtime, 0, PIDTaskStackSize, PIDTaskBuffer, &PIDTaskControlBlock);
  PIDTaskHandle = osThreadCreate(osThread(PIDTask), NULL);

  /* definition and creation of MOVTask - Accelerometer management */
  osThreadStaticDef(MOVTask, startMOVTask, osPriorityNormal, 0, MOVTaskStackSize, MOVTaskBuffer, &MOVTaskControlBlock);
  MOVTaskHandle = osThreadCreate(osThread(MOVTask), NULL);
  resetWatchdog();

  /* Start scheduler */
  osKernelStart();
  /* We should never get here as control is now taken by the scheduler */
  for (;;) {}
}