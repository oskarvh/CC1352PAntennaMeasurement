/*
 *  ======== ti_radio_config.h ========
 *  Configured RadioConfig module definitions
 *
 *  DO NOT EDIT - This file is generated for the CC1352P1F3RGZ
 *  by the SysConfig tool.
 *
 *  Radio Config module version : 1.13
 *  SmartRF Studio data version : 2.25.0
 */
#ifndef _TI_RADIO_CONFIG_H_
#define _TI_RADIO_CONFIG_H_

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include <ti/drivers/rf/RF.h>

/* SmartRF Studio version that the RF data is fetched from */
#define SMARTRF_STUDIO_VERSION "2.25.0"

// *********************************************************************************
//   RF Frontend configuration
// *********************************************************************************
// RF design based on: LAUNCHXL-CC1352P-2 (LAUNCHXL-CC1352P2_20dBm)
#define LAUNCHXL_CC1352P_2

// High-Power Amplifier supported
#define SUPPORT_HIGH_PA

// RF frontend configuration
#define FRONTEND_SUB1G_DIFF_RF
#define FRONTEND_SUB1G_EXT_BIAS
#define FRONTEND_24G_DIFF_RF
#define FRONTEND_24G_EXT_BIAS

// Supported frequency bands
#define SUPPORT_FREQBAND_868
#define SUPPORT_FREQBAND_2400

// TX power table size definitions
#define TXPOWERTABLE_868_PA13_SIZE 22 // 868 MHz, 13 dBm
#define TXPOWERTABLE_2400_PA5_SIZE 16 // 2400 MHz, 5 dBm
#define TXPOWERTABLE_2400_PA20_SIZE 8 // 2400 MHz, 20 dBm

// TX power tables
extern RF_TxPowerTable_Entry txPowerTable_868_pa13[]; // 868 MHz, 13 dBm
extern RF_TxPowerTable_Entry txPowerTable_2400_pa5[]; // 2400 MHz, 5 dBm
extern RF_TxPowerTable_Entry txPowerTable_2400_pa20[]; // 2400 MHz, 20 dBm



//*********************************************************************************
//  RF Setting:   Custom (100 kbps, 50 kHz Deviation, 2-GFSK, 311 kHz RX Bandwidth)
//
//  PHY:          custom2400
//  Setting file: setting_tc900_custom.json
//*********************************************************************************

// PA table usage
#define TX_POWER_TABLE_SIZE_custom2400_0 TXPOWERTABLE_2400_PA5_SIZE

#define txPowerTable_custom2400_0 txPowerTable_2400_pa5

// TI-RTOS RF Mode object
extern RF_Mode RF_prop_custom2400_1;

// RF Core API commands
extern rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup_custom2400_1;
extern rfc_CMD_FS_t RF_cmdFs_custom2400_1;
extern rfc_CMD_PROP_TX_t RF_cmdPropTx_custom2400_1;
extern rfc_CMD_PROP_RX_t RF_cmdPropRx_custom2400_1;

// RF Core API overrides
extern uint32_t pOverrides_custom2400_1[];

#endif // _TI_RADIO_CONFIG_H_
