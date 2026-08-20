/*===========================================================================
* Copyright (c) 2018-2023 Qualcomm Technologies Incorporated. All Rights Reserved.
* QUALCOMM Confidential and Proprietary
*
*$Header: //components/rel/core.qdsp6/6.0/battman/pmic/platform/src/pm_peripheral_chgr.c#17 $
*$DateTime: 2023/06/16 06:07:47 $
*$Author: pwbldsvc $
=============================================================================
EDIT HISTORY

when           who     what, where, why
--------       ---     -----------------------------------------------------------
11/13/2019     ravi    Added support for PM8350B
12/03/2018     dch     Updated correct register mask for Pre-Charge Current configuration
11/19/2018     ivy     Cap Max Float Voltage and Max Charge Current for SMB2351V2.0 only  
10/02/2018     cs      added api to get charger enabled status
08/24/2018     dch     Fix Incorrect Float Voltage capping logic
08/20/2018     dch     Cap Max Float Votlage to 8.5V, Max Charge Current to 3A for SMB2351V2
05/23/2018     cs      when invalid safety Timer Value, disable charger safety Timer.
04/26/2018     ivy     Added new API
04/13/2018     ivy     Inital Release
=============================================================================*/

#include "pm_peripheral_chgr.h"
#include "pm_interface.h"
#include "pm_register_scpq_p_chgr.h"

#define SCHGCHGRFUNCPTR_PM855B 0
#define SCHGCHGRFUNCPTR_SMB2351 1
#define SCHGCHGRFUNCPTR_PM8350B 2
#define SCHGCHGRFUNCPTR_PM7325B 3
#define SCHGCHGRFUNCPTR_PMI632 4
#define FLOAT_VOLT_STEP_CONSTANT 20
#define FLOAT_VOLT_STEP_CONSTANT_PM8150B 10
#define FLOAT_VOLT_STEP_CONSTANT_PM8350B 10
#define FLOAT_VOLT_MIN 7200
#define FLOAT_VOLT_MAX 9000
#define FLOAT_VOLT_MAX_SMB2351V2 8500
#define FLOAT_VOLT_MIN_PM8150B 3600
#define FLOAT_VOLT_MAX_PM8150B 4790
#define FLOAT_VOLT_MIN_PM8350B 3600
#define FLOAT_VOLT_MAX_PM8350B 4790
#define FLOAT_VOLT_MIN_PM7250B 3600
#define FLOAT_VOLT_MAX_PM7250B 4790
#define PRE_CHARGE_CURRENT_STEP_SIZE 50
#define PRE_CHARGE_CURRENT_MAX 1000
#define PRE_CHARGE_CURRENT_MIN 50
#define PRE_CHARGE_CURRENT_MAX_PM8350B 450
#define PRE_CHARGE_CURRENT_MIN_PM8350B 100
#define PRE_CHARGE_CURRENT_MAX_PM855B_PM7250B 450
#define PRE_CHARGE_CURRENT_MIN_PM855B_PM7250B 100
#define FAST_CHARGE_CURRENT_STEP_SIZE 50
#define FAST_CHARGE_CURRENT_MAX 6000
#define FAST_CHARGE_CURRENT_MAX_SMB2351V2 3000
#define FAST_CHARGE_CURRENT_MAX_PM8150B 8000
#define FAST_CHARGE_CURRENT_MAX_PM8350B 12000
#define FAST_CHARGE_CURRENT_MAX_PM7250B 12000
#define FAST_CHARGE_CURRENT_MAX_PMI632 3000
#define FAST_CHARGE_CURRENT_MIN_PM8350B 0
#define FAST_CHARGE_CURRENT_MIN 0
#define TERMINATION_CURRENT_STEP_SIZE 50
#define TERMINATION_CURRENT_MAX 750
#define TERMINATION_CURRENT_MIN 0
#define TERMINATION_CURRENT_MAX_PM8350B  450
#define TERMINATION_CURRENT_MIN_PM8350B  100

#define ANALOG_TERMINATION_CURRENT_MAX 350
#define DIGITAL_TERMINATON_CURRENT_MAX 10000
#define DIGITAL_TERMINATON_CURRENT_MAX_PMI632 5000
#define INT16_MAX_VALUE 32767
#define ANALOG_TERMINATION_CURRENT_MAX_PM855B_PM7250B 450

PM_SCHG_CHGR_CONFIG_TYPE gSchgChgrConfig[PMIC_INDEX_MAX] = {};

PM_SCHG_CHGR_FUNC_PTR_TYPE SchgChgrFuncPtr[] =
{
    {
        //PM855B and PM7250B
        .SetChgrCurrent = schg_chgr_set_charge_current_rev0,
        .SetEnableChgr = schg_chgr_set_enable_charging_rev1,
        .GetChgrStatus = schg_chgr_get_chgr_status_rev1,
        .SetFloatVolt = schg_chgr_set_float_volt_rev0,
        .GetFloatVolt = schg_chgr_get_float_volt_rev0,
        .SetJeitaFvcompCfg = schg_chgr_set_jeita_fvcomp_cfg_rev1,
        .SetJeitaCccomp = schg_chgr_set_jeita_cccomp_rev1,
        .SetJeitaThreshCfg = schg_chgr_set_jeita_threshold_value_rev1,
        .GetJeitaThreshCfg = schg_chgr_get_jeita_threshold_value_rev1,
        .SetJeitaEnCfg = schg_chgr_set_jeita_en_cfg_rev1,
        .GetBattTempStatus = schg_chgr_get_batt_temp_status_rev1,
        .SetFastChargeSafetyTimerCfg = schg_chgr_set_fast_charge_safety_timer_cfg_rev1,
        .SetChgrCfg2 = schg_chgr_set_chgr_cfg2_rev1,
        .SetChargeInhibitThresholdCfg = schg_chgr_set_charge_inhibit_threshold_cfg_rev1,
        .GetEnableChgr = schg_chgr_get_enable_charging_rev1,
        .GetChgrStatusR2 = schg_chgr_get_chgr_status_r2_rev1,
        .SetDigIterm = schg_chgr_set_dig_term_current_rev1,
        .SetSmbEnPassthroughControl = NULL,
    },

    {
        // SMB2351
        .SetChgrCurrent = schg_chgr_set_charge_current_rev1,
        .SetEnableChgr = schg_chgr_set_enable_charging_rev1,
        .GetChgrStatus = schg_chgr_get_chgr_status_rev1,
        .SetFloatVolt = schg_chgr_set_float_volt_rev1,
        .GetFloatVolt = NULL,
        .SetJeitaFvcompCfg = schg_chgr_set_jeita_fvcomp_cfg_rev1,
        .SetJeitaCccomp = schg_chgr_set_jeita_cccomp_rev1,
        .SetJeitaThreshCfg = schg_chgr_set_jeita_threshold_value_rev1,
        .GetJeitaThreshCfg = NULL,
        .SetJeitaEnCfg = schg_chgr_set_jeita_en_cfg_rev1,
        .GetBattTempStatus = NULL,
        .SetFastChargeSafetyTimerCfg = schg_chgr_set_fast_charge_safety_timer_cfg_rev1,
        .SetChgrCfg2 = schg_chgr_set_chgr_cfg2_rev1,
        .SetChargeInhibitThresholdCfg = schg_chgr_set_charge_inhibit_threshold_cfg_rev1,
        .GetEnableChgr = schg_chgr_get_enable_charging_rev1,
        .GetChgrStatusR2 = NULL,
    	.SetDigIterm = NULL,
        .SetSmbEnPassthroughControl = NULL,
    },

    {
        // PM8350B
        .SetChgrCurrent = schg_chgr_set_charge_current_rev2,
        .SetEnableChgr = schg_chgr_set_enable_charging_rev2,
        .GetChgrStatus = NULL,
        .SetFloatVolt = schg_chgr_set_float_volt_rev2,
        .GetFloatVolt = schg_chgr_get_float_volt_rev2,
        .SetJeitaFvcompCfg = NULL,
        .SetJeitaCccomp = NULL,
        .SetJeitaThreshCfg = NULL,
        .GetJeitaThreshCfg = NULL,
        .SetJeitaEnCfg = NULL,
        .GetBattTempStatus = NULL,
        .SetFastChargeSafetyTimerCfg = schg_chgr_set_fast_charge_safety_timer_cfg_rev2,
        .SetChgrCfg2 = schg_chgr_set_chgr_cfg2_rev2,
        .SetChargeInhibitThresholdCfg = schg_chgr_set_charge_inhibit_threshold_cfg_rev2,
        .GetEnableChgr = schg_chgr_get_enable_charging_rev2,
        .GetChgrStatusR2 = schg_chgr_get_chgr_status_r2_rev0,
        .SetDigIterm = schg_chgr_set_dig_term_current,
        .SetSmbEnPassthroughControl = schg_smb_en_passtrough_control_rev2,
    },

    {
        // PM7325B
        .SetChgrCurrent = schg_chgr_set_charge_current_rev3,
        .SetEnableChgr = schg_chgr_set_enable_charging_rev2,
        .GetChgrStatus = NULL,
        .SetFloatVolt = schg_chgr_set_float_volt_rev2,
        .GetFloatVolt = schg_chgr_get_float_volt_rev2,
        .SetJeitaFvcompCfg = NULL,
        .SetJeitaCccomp = NULL,
        .SetJeitaThreshCfg = NULL,
        .GetJeitaThreshCfg = NULL,
        .SetJeitaEnCfg = NULL,
        .GetBattTempStatus = NULL,
        .SetFastChargeSafetyTimerCfg = schg_chgr_set_fast_charge_safety_timer_cfg_rev2,
        .SetChgrCfg2 = schg_chgr_set_chgr_cfg2_rev2,
        .SetChargeInhibitThresholdCfg = schg_chgr_set_charge_inhibit_threshold_cfg_rev2,
        .GetEnableChgr = schg_chgr_get_enable_charging_rev2,
        .GetChgrStatusR2 = schg_chgr_get_chgr_status_r2_rev0,
        .SetDigIterm = schg_chgr_set_dig_term_current,
        .SetSmbEnPassthroughControl = schg_smb_en_passtrough_control_rev2,
    },

    {
        // PMI632
        .SetChgrCurrent = schg_chgr_set_charge_current_rev0,
        .SetEnableChgr = schg_chgr_set_enable_charging_rev1,
        .GetChgrStatus = schg_chgr_get_chgr_status_rev1,
        .SetFloatVolt = schg_chgr_set_float_volt_rev0,
        .GetFloatVolt = schg_chgr_get_float_volt_rev0,
        .SetJeitaFvcompCfg = schg_chgr_set_jeita_fvcomp_cfg_rev1,
        .SetJeitaCccomp = schg_chgr_set_jeita_cccomp_rev1,
        .SetJeitaThreshCfg = schg_chgr_set_jeita_threshold_value_rev1,
        .GetJeitaThreshCfg = NULL,
        .SetJeitaEnCfg = schg_chgr_set_jeita_en_cfg_rev1,
        .GetBattTempStatus = schg_chgr_get_batt_temp_status_rev1,
        .SetFastChargeSafetyTimerCfg = schg_chgr_set_fast_charge_safety_timer_cfg_rev1,
        .SetChgrCfg2 = schg_chgr_set_chgr_cfg2_rev1,
        .SetChargeInhibitThresholdCfg = schg_chgr_set_charge_inhibit_threshold_cfg_rev1,
        .GetEnableChgr = schg_chgr_get_enable_charging_rev1,
        .GetChgrStatusR2 = schg_chgr_get_chgr_status_r2_rev1,
        .SetDigIterm = schg_chgr_set_dig_term_current_rev2,
        .SetSmbEnPassthroughControl = NULL,
    }

};

PMSTATUS InitializePeripheralSchgChgr(PMIC_INDEX_TYPE PmicIndex, PPMIC_INFO_TYPE pPmicInfo)
{
    if (PmicIndex >= PMIC_INDEX_INVALID || pPmicInfo == NULL)
    {
        TraceErr(BattMngrWPP_SelfHost, "InitializePeripheralSchgChgr: STATUS_ERROR_INVALID_ARGUMENT");
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    PPM_SCHG_CHGR_CONFIG_TYPE SchgChgrConfig = &gSchgChgrConfig[PmicIndex];

    if (SchgChgrConfig == NULL)
    {
        TraceErr(BattMngrWPP_SelfHost, "InitializePeripheralSchgChgr: STATUS_ERROR_INVALID_ARGUMENT");
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    switch (pPmicInfo->PmicModel)
    {
        case PMIC_MODEL_SMB2351:
            SchgChgrConfig->FuncPtr = &SchgChgrFuncPtr[SCHGCHGRFUNCPTR_SMB2351];
            //Load Hardware Revision Specific configuration based on PMIC Info

            //For V2.0 SMB2351
            if (pPmicInfo->PmicRevision.AllLayers == 2 && pPmicInfo->PmicRevision.Metal == 0)
            {
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX]          = FLOAT_VOLT_MAX_SMB2351V2;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX] = FAST_CHARGE_CURRENT_MAX_SMB2351V2;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MIN]          = FLOAT_VOLT_MIN;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_STEP]         = FLOAT_VOLT_STEP_CONSTANT;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_MIN]        = TERMINATION_CURRENT_MIN;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_STEP]       = TERMINATION_CURRENT_STEP_SIZE;
            }
            //For Other revision
            else
            {
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX]          = FLOAT_VOLT_MAX;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX] = FAST_CHARGE_CURRENT_MAX;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MIN]          = FLOAT_VOLT_MIN;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_STEP]         = FLOAT_VOLT_STEP_CONSTANT;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_MIN]        = TERMINATION_CURRENT_MIN;
                SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_STEP]       = TERMINATION_CURRENT_STEP_SIZE;
            }
            break;
        case PMIC_MODEL_PM8150B:
            SchgChgrConfig->FuncPtr = &SchgChgrFuncPtr[SCHGCHGRFUNCPTR_PM855B];
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX]              = FLOAT_VOLT_MAX_PM8150B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX]     = FAST_CHARGE_CURRENT_MAX_PM8150B;            
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MIN]              = FLOAT_VOLT_MIN_PM8150B;                            
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_STEP]             = FLOAT_VOLT_STEP_CONSTANT_PM8150B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_MIN]            = TERMINATION_CURRENT_MIN;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_STEP]           = TERMINATION_CURRENT_STEP_SIZE;
            break;
        case PMIC_MODEL_PM8350B:
            SchgChgrConfig->FuncPtr = &SchgChgrFuncPtr[SCHGCHGRFUNCPTR_PM8350B];
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX]              = FLOAT_VOLT_MAX_PM8350B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX]     = FAST_CHARGE_CURRENT_MAX_PM8350B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MIN]              = FLOAT_VOLT_MIN_PM8350B; 
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_STEP]             = FLOAT_VOLT_STEP_CONSTANT_PM8350B;      
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_MIN]            = TERMINATION_CURRENT_MIN_PM8350B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_STEP]           = TERMINATION_CURRENT_STEP_SIZE;
            break;
        case PMIC_MODEL_PM6450A:
        case PMIC_MODEL_PM7325B:
            SchgChgrConfig->FuncPtr = &SchgChgrFuncPtr[SCHGCHGRFUNCPTR_PM7325B];
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX]              = FLOAT_VOLT_MAX_PM8350B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX]     = FAST_CHARGE_CURRENT_MAX_PM8350B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MIN]              = FLOAT_VOLT_MIN_PM8350B; 
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_STEP]             = FLOAT_VOLT_STEP_CONSTANT_PM8350B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_MIN]            = TERMINATION_CURRENT_MIN_PM8350B;
            SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_STEP]           = TERMINATION_CURRENT_STEP_SIZE;
            break;
        case PMIC_MODEL_PMI632:
            SchgChgrConfig->FuncPtr = &SchgChgrFuncPtr[SCHGCHGRFUNCPTR_PMI632];
			SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX] = FLOAT_VOLT_MAX_PM7250B;
			SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX] = FAST_CHARGE_CURRENT_MAX_PMI632;
            break;
		case PMIC_MODEL_PM7250B:
			SchgChgrConfig->FuncPtr = &SchgChgrFuncPtr[SCHGCHGRFUNCPTR_PM855B];
			SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX] = FLOAT_VOLT_MAX_PM7250B;
			SchgChgrConfig->HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX] = FAST_CHARGE_CURRENT_MAX_PM7250B;
		break;
        default:
            TraceErr(BattMngrWPP_SelfHost, "InitializePeripheralSchgChgr: PmicModel = %d not supported for SchgChgr peripheral", pPmicInfo->PmicModel);
            return STATUS_ERROR_NOT_SUPPORTED;
    }

    SchgChgrConfig->PeripheralSlaveId = pPmicInfo->SpmiSlaveId[PMIC_SLAVEID_INDEX_0];
    SchgChgrConfig->IsInitialized = true;

    return STATUS_SUCCESS;
}

PMSTATUS DeinitializePeripheralSchgChgr(PMIC_INDEX_TYPE PmicIndex)
{
    PPM_SCHG_CHGR_CONFIG_TYPE SchgChgrConfig = &gSchgChgrConfig[PmicIndex];

    if (SchgChgrConfig == NULL)
    {
        TraceErr(BattMngrWPP_SelfHost, "DeinitializePeripheralSchgChgr: STATUS_ERROR_INVALID_ARGUMENT");
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    if (SchgChgrConfig->IsInitialized)
    {
        SchgChgrConfig->FuncPtr = NULL;
    }

    SchgChgrConfig->IsInitialized = false;

    return STATUS_SUCCESS;
}

//
// HW Specific functions
//
PMSTATUS schg_chgr_set_charge_current_rev1(UINT8 pmic_index, PM_CHGR_CURRUENT_TYPE *charge_current_type, uint32 current)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_ADDRESS_TYPE reg = 0;
    PMIC_REGISTER_DATA_TYPE mask = 0;
    PMIC_REGISTER_DATA_TYPE data = 0;
    uint32 step_size = 0;
    uint32 max_value = 0;
    uint32 min_value = 0;

    if (pmic_index >= PMIC_INDEX_INVALID || charge_current_type == NULL)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    switch (*charge_current_type)
    {
        case CHGR_PRE_CHARGE_CURRENT:
            reg = SCHG_CHGR_PRE_CHARGE_CURRENT_CFG;
            mask = SCHG_CHGR_PRE_CHARGE_CURRENT_CFG_MASK; // BIT<4:0>
            step_size = PRE_CHARGE_CURRENT_STEP_SIZE;
            max_value = PRE_CHARGE_CURRENT_MAX;
            min_value = PRE_CHARGE_CURRENT_MIN;
            break;

        case CHGR_FAST_CHARGE_CURRENT:
            reg = SCHG_CHGR_FAST_CHARGE_CURRENT;
            mask = SCHGP_CHGR_FAST_CHARGE_CURRENT_MASK;
            step_size = FAST_CHARGE_CURRENT_STEP_SIZE;
            max_value = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX];
            min_value = FAST_CHARGE_CURRENT_MIN;
            break;

        case CHGR_TERMINATION_CURRENT:
            reg = SCHG_CHGR_TCCC_CHARGE_CURRENT_TERMINATION_CFG;
            mask = SCHG_CHGR_TCCC_CHARGE_CURRENT_TERMINATION_CFG_MASK; // BIT<3:0>
            step_size = TERMINATION_CURRENT_STEP_SIZE;
            max_value = TERMINATION_CURRENT_MAX;
            min_value = TERMINATION_CURRENT_MIN;
            break;
        case CHG_TERMINATION_ANALOG_CURRENT:
            reg = SCHG_CHGR_CHARGE_CURRENT_TERMINATION_CFG;
            mask = SCHG_CHGR_CHARGE_CURRENT_TERMINATION_CFG_MASK; // BIT<2:0>
            step_size = TERMINATION_CURRENT_STEP_SIZE;
            max_value = ANALOG_TERMINATION_CURRENT_MAX;
            min_value = TERMINATION_CURRENT_MIN;
            if (current >= TERMINATION_CURRENT_STEP_SIZE)
            {
                current -= TERMINATION_CURRENT_STEP_SIZE; //for analog termination current 0 means 50 mA,
            }
            else
                current = 0; //less than 50 also set to 50
            break;
        default:
            return STATUS_ERROR_NOT_SUPPORTED;
            break;
    }

    if (current < min_value)
    {
        current = min_value;
    }
    else if (current > max_value)
    {
        current = max_value;
    }

    /*rounding and getting corresponding register value*/
    data = (current + (step_size / 2)) / step_size;
    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, reg, mask, data);

    return status;
}


PMSTATUS schg_chgr_set_charge_current_rev0(UINT8 pmic_index, PM_CHGR_CURRUENT_TYPE *charge_current_type, uint32 current)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_ADDRESS_TYPE reg = 0;
    PMIC_REGISTER_DATA_TYPE mask = 0;
    PMIC_REGISTER_DATA_TYPE data = 0;
    uint32 step_size = 0;
    uint32 max_value = 0;
    uint32 min_value = 0;

    if (pmic_index >= PMIC_INDEX_INVALID || charge_current_type == NULL)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    switch (*charge_current_type)
    {
        case CHGR_PRE_CHARGE_CURRENT:
            reg = SCHG_CHGR_PRE_CHARGE_CURRENT_CFG;
            mask = SCHG_CHGR_PRE_CHARGE_CURRENT_CFG_MASK; // BIT<4:0>
            step_size = PRE_CHARGE_CURRENT_STEP_SIZE;
            max_value = PRE_CHARGE_CURRENT_MAX_PM855B_PM7250B;
            min_value = PRE_CHARGE_CURRENT_MIN_PM855B_PM7250B;
            break;

        case CHGR_FAST_CHARGE_CURRENT:
            reg = SCHG_CHGR_FAST_CHARGE_CURRENT;
            mask = SCHG_CHGR_FAST_CHARGE_CURRENT_MASK; // BIT<7:0>
            step_size = FAST_CHARGE_CURRENT_STEP_SIZE;
            max_value = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX];
            min_value = FAST_CHARGE_CURRENT_MIN;
            break;

        case CHGR_TERMINATION_CURRENT:
            reg = SCHG_CHGR_TCCC_CHARGE_CURRENT_TERMINATION_CFG;
            mask = SCHG_CHGR_TCCC_CHARGE_CURRENT_TERMINATION_CFG_MASK; // BIT<3:0>
            step_size = TERMINATION_CURRENT_STEP_SIZE;
            max_value = TERMINATION_CURRENT_MAX;
            min_value = TERMINATION_CURRENT_MIN;
            break;
        case CHG_TERMINATION_ANALOG_CURRENT:
            reg = SCHG_CHGR_CHARGE_CURRENT_TERMINATION_CFG;
            mask = SCHG_CHGR_CHARGE_CURRENT_TERMINATION_CFG_MASK; // BIT<2:0>
            step_size = TERMINATION_CURRENT_STEP_SIZE;
            max_value = ANALOG_TERMINATION_CURRENT_MAX_PM855B_PM7250B;
            min_value = TERMINATION_CURRENT_MIN;
            
            #define TERMINATION_CURRENT_MIN_VAL 100
            if (current >= TERMINATION_CURRENT_MIN_VAL)
            {
                current -= TERMINATION_CURRENT_MIN_VAL; //for analog termination current 0 means 100 mA,
            }
            else
                current = 0; //less than 50 also set to 50
            break;
        default:
            return STATUS_ERROR_NOT_SUPPORTED;
            break;
    }

    if (current < min_value)
    {
        current = min_value;
    }
    else if (current > max_value)
    {
        current = max_value;
    }

    /*getting corresponding register value*/
    data = current / step_size;
    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, reg, mask, data);

    return status;
}

PMSTATUS schg_chgr_set_charge_current_rev2(UINT8 pmic_index, PM_CHGR_CURRUENT_TYPE *charge_current_type, uint32 current)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_ADDRESS_TYPE reg = 0;
    PMIC_REGISTER_DATA_TYPE mask = 0;
    PMIC_REGISTER_DATA_TYPE data = 0;
    PMIC_REGISTER_DATA_TYPE cfg_2s = 0;

    uint32 step_size = 0;
    uint32 max_value = 0;
    uint32 min_value = 0;
    uint32 base0_value = 0;

    if (pmic_index >= PMIC_INDEX_INVALID || charge_current_type == NULL)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_IN_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_BAT_2S_CHARGE_CFG_ADDR, PMIO_SCPQ_P_CHGR_BAT_2S_CHARGE_CFG_BAT_2S_CHARGE_CFG_BMSK, &cfg_2s);
    if (status != STATUS_SUCCESS)
        return status;

    switch (*charge_current_type)
    {
        case CHGR_PRE_CHARGE_CURRENT:
            reg = PMIO_SCPQ_P_CHGR_IPRE_CFG_ADDR;
            mask = PMIO_SCPQ_P_CHGR_IPRE_CFG_PRE_CHARGE_CURRENT_SETTING_BMSK;
            step_size = PRE_CHARGE_CURRENT_STEP_SIZE;
            max_value = PRE_CHARGE_CURRENT_MAX_PM8350B;
            min_value = PRE_CHARGE_CURRENT_MIN_PM8350B;
            base0_value = PRE_CHARGE_CURRENT_MIN_PM8350B; //value 0 is 100mA
            break;

        case CHGR_FAST_CHARGE_CURRENT:
            reg = PMIO_SCPQ_P_CHGR_ICHG_CFG_ADDR;
            mask = PMIO_SCPQ_P_CHGR_ICHG_CFG_FAST_CHARGE_CURRENT_SETTING_BMSK;
            step_size = FAST_CHARGE_CURRENT_STEP_SIZE;
            max_value = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX];
            min_value = FAST_CHARGE_CURRENT_MIN_PM8350B;
            base0_value = FAST_CHARGE_CURRENT_MIN_PM8350B; //value 0 is 0mA
            break;
        
        case CHG_TERMINATION_ANALOG_CURRENT:
            data = PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_USE_ANALOG_TERMINATION_BMSK;

        case CHGR_TERMINATION_CURRENT:
            reg = PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_ADDR;
            mask = (PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_ANALOG_CHARGE_CURRENT_TERMINATION_SETTING_BMSK | 
                    PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_USE_ANALOG_TERMINATION_BMSK);

            step_size = TERMINATION_CURRENT_STEP_SIZE;
            max_value = TERMINATION_CURRENT_MAX_PM8350B;
            min_value = TERMINATION_CURRENT_MIN_PM8350B;
            base0_value = TERMINATION_CURRENT_MIN_PM8350B; //value 0 is 100mA
            break;

        default:
            return STATUS_ERROR_NOT_SUPPORTED;
            break;
    }

    //if reg value 0 is valid, adjust the current value
    if (current >= base0_value)
    {
        current -= base0_value;
    }
    else
        current = 0;

    //if reg value 0 is valid, update max_value before comparision
    if (base0_value)
    {
        max_value -= base0_value;
        min_value -= base0_value;
    }

    if (current < min_value)
    {
        current = min_value;
    }
    else if (current > max_value)
    {
        current = max_value;
    }

    /*getting corresponding register value*/
    data |= current / step_size;
    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, reg, mask, data);

    return status;
}

PMSTATUS schg_chgr_set_charge_current_rev3(UINT8 pmic_index, PM_CHGR_CURRUENT_TYPE *charge_current_type, uint32 current)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_ADDRESS_TYPE reg = 0;
    PMIC_REGISTER_DATA_TYPE mask = 0;
    PMIC_REGISTER_DATA_TYPE data = 0;

    uint32 step_size = 0;
    uint32 max_value = 0;
    uint32 min_value = 0;
    uint32 base0_value = 0;

    if (pmic_index >= PMIC_INDEX_INVALID || charge_current_type == NULL)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    switch (*charge_current_type)
    {
        case CHGR_PRE_CHARGE_CURRENT:
            reg = PMIO_SCPQ_P_CHGR_IPRE_CFG_ADDR;
            mask = PMIO_SCPQ_P_CHGR_IPRE_CFG_PRE_CHARGE_CURRENT_SETTING_BMSK;
            step_size = PRE_CHARGE_CURRENT_STEP_SIZE;
            max_value = PRE_CHARGE_CURRENT_MAX_PM8350B;
            min_value = PRE_CHARGE_CURRENT_MIN_PM8350B;
            base0_value = PRE_CHARGE_CURRENT_MIN_PM8350B; //value 0 is 100mA
            break;

        case CHGR_FAST_CHARGE_CURRENT:
            reg = PMIO_SCPQ_P_CHGR_ICHG_CFG_ADDR;
            mask = PMIO_SCPQ_P_CHGR_ICHG_CFG_FAST_CHARGE_CURRENT_SETTING_BMSK;
            step_size = FAST_CHARGE_CURRENT_STEP_SIZE;
            max_value = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FAST_CHARGE_CURRENT_MAX];
            min_value = FAST_CHARGE_CURRENT_MIN_PM8350B;
            base0_value = FAST_CHARGE_CURRENT_MIN_PM8350B; //value 0 is 0mA
            break;
        
        case CHG_TERMINATION_ANALOG_CURRENT:
            data = PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_USE_ANALOG_TERMINATION_BMSK;
            break;

        case CHGR_TERMINATION_CURRENT:
            reg = PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_ADDR;
            mask = (PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_ANALOG_CHARGE_CURRENT_TERMINATION_SETTING_BMSK | 
                    PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_USE_ANALOG_TERMINATION_BMSK);

            step_size = TERMINATION_CURRENT_STEP_SIZE;
            max_value = TERMINATION_CURRENT_MAX_PM8350B;
            min_value = TERMINATION_CURRENT_MIN_PM8350B;
            base0_value = TERMINATION_CURRENT_MIN_PM8350B; //value 0 is 100mA
            break;

        default:
            return STATUS_ERROR_NOT_SUPPORTED;
            break;
    }

    //if reg value 0 is valid, adjust the current value
    if (current >= base0_value)
    {
        current -= base0_value;
    }
    else
        current = 0;

    //if reg value 0 is valid, update max_value before comparision
    if (base0_value)
    {
        max_value -= base0_value;
        min_value -= base0_value;
    }

    if (current < min_value)
    {
        current = min_value;
    }
    else if (current > max_value)
    {
        current = max_value;
    }

    /*getting corresponding register value*/
    data |= current / step_size;
    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, reg, mask, data);

    return status;
}

PMSTATUS schg_chgr_set_enable_charging_rev1(UINT8 pmic_index, BOOLEAN enable)
{
    PMSTATUS status = STATUS_SUCCESS;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_CHARGING_ENABLE_CMD, SCHG_CHGR_CHARGING_ENABLE_CMD_MASK, (PMIC_REGISTER_DATA_TYPE)enable);

    return status;
}

PMSTATUS schg_chgr_set_enable_charging_rev2(UINT8 pmic_index, BOOLEAN enable)
{
    PMSTATUS status = STATUS_SUCCESS;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                            PMIO_SCPQ_P_CHGR_CHG_EN_ADDR,
                            PMIO_SCPQ_P_CHGR_CHG_EN_CHARGING_ENABLE_CMD_BMSK,
                            (PMIC_REGISTER_DATA_TYPE)enable);

    return status;
}

PMSTATUS schg_chgr_get_enable_charging_rev1(UINT8 pmic_index, BOOLEAN *enable)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data;

    if (pmic_index >= PMIC_INDEX_INVALID || NULL == enable)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_IN_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_CHARGING_ENABLE_CMD, SCHG_CHGR_CHARGING_ENABLE_CMD_MASK, &data);

    if (status == STATUS_SUCCESS)
    {
        *enable = (BOOLEAN)data;
    }

    return status;
}

PMSTATUS schg_chgr_get_enable_charging_rev2(UINT8 pmic_index, BOOLEAN *enable)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data;

    if (pmic_index >= PMIC_INDEX_INVALID || NULL == enable)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_IN_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                        PMIO_SCPQ_P_CHGR_CHG_EN_ADDR,
                        PMIO_SCPQ_P_CHGR_CHG_EN_CHARGING_ENABLE_CMD_BMSK,
                        &data);

    if (status == STATUS_SUCCESS)
    {
        *enable = (BOOLEAN)data;
    }

    return status;
}

PMSTATUS schg_chgr_get_chgr_status_rev1(UINT8 pmic_index, PM_CHGR_CHGR_STATUS_TYPE *charger_status)
{
    PMSTATUS status = STATUS_SUCCESS;

    // Create two arrays to store data for status_1,2 and status_3,4,5,6,7,8
    // Between status 2 and status 3 there is gap, hence breaking
    PMIC_REGISTER_DATA_TYPE data1[2] = {0};
    PMIC_REGISTER_DATA_TYPE data2[6] = {0};

    if (pmic_index >= PMIC_INDEX_INVALID || NULL == charger_status)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    // read status_1 and status_2
    status |= PM_IN_MULTIBYTES(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_BATTERY_CHARGER_STATUS_1, 2, data1);

    // read status_3,4,5,6,7,8
    status |= PM_IN_MULTIBYTES(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_BATTERY_CHARGER_STATUS_3, 6, data2);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    /*status 1 related*/
    charger_status->status_1.battery_charger_status = data1[0] & (PMIC_REGISTER_BIT_0 | PMIC_REGISTER_BIT_1 | PMIC_REGISTER_BIT_2);
    charger_status->status_1.step_charging_status = (data1[0] & (PMIC_REGISTER_BIT_3 | PMIC_REGISTER_BIT_4 | PMIC_REGISTER_BIT_5)) >> 3;
    charger_status->status_1.zero_charging_current = (data1[0] & PMIC_REGISTER_BIT_6) ? TRUE : FALSE;
    charger_status->status_1.icl_incr_req_for_prechg = (data1[0] & PMIC_REGISTER_BIT_7) ? TRUE : FALSE;

    /*status 2 related*/
    charger_status->status_2.bat_therm_missing = (data1[1] & PMIC_REGISTER_BIT_0) ? TRUE : FALSE;
    charger_status->status_2.bat_ov = (data1[1] & PMIC_REGISTER_BIT_1) ? TRUE : FALSE;
    charger_status->status_2.bat_sft_expire = (data1[1] & PMIC_REGISTER_BIT_2) ? TRUE : FALSE;
    charger_status->status_2.bat_gt_full_on = (data1[1] & PMIC_REGISTER_BIT_3) ? TRUE : FALSE;
    charger_status->status_2.vbatt_gtet_float_voltage = (data1[1] & PMIC_REGISTER_BIT_4) ? TRUE : FALSE;
    charger_status->status_2.vbatt_gtet_inhibit = (data1[1] & PMIC_REGISTER_BIT_5) ? TRUE : FALSE;
    charger_status->status_2.vbatt_ltet_recharge = (data1[1] & PMIC_REGISTER_BIT_6) ? TRUE : FALSE;
    charger_status->status_2.drop_in_vbat_reference = (data1[1] & PMIC_REGISTER_BIT_7) ? TRUE : FALSE;

    /*status 3 related*/
    charger_status->status_3 = data2[0];

    /*status 4 related*/
    charger_status->status_4 = data2[1];

    /*status 5 related*/
    charger_status->status_5.enable_fullon_mode = (data2[2] & PMIC_REGISTER_BIT_0) ? TRUE : FALSE;
    charger_status->status_5.enable_pre_charging = (data2[2] & PMIC_REGISTER_BIT_1) ? TRUE : FALSE;
    charger_status->status_5.enable_trickle = (data2[2] & PMIC_REGISTER_BIT_2) ? TRUE : FALSE;
    charger_status->status_5.enable_chg_sensors = (data2[2] & PMIC_REGISTER_BIT_3) ? TRUE : FALSE;
    charger_status->status_5.charging_enable = (data2[2] & PMIC_REGISTER_BIT_4) ? TRUE : FALSE;
    charger_status->status_5.force_zero_charge_current = (data2[2] & PMIC_REGISTER_BIT_5) ? TRUE : FALSE;
    charger_status->status_5.disable_charging = (data2[2] & PMIC_REGISTER_BIT_6) ? TRUE : FALSE;
    charger_status->status_5.valid_input_power_source = (data2[2] & PMIC_REGISTER_BIT_7) ? TRUE : FALSE;

    /*status 6 related*/
    charger_status->status_6.jeita_adc_thot_afp = (data2[3] & PMIC_REGISTER_BIT_0) ? TRUE : FALSE;
    charger_status->status_6.jeita_adc_thot = (data2[3] & PMIC_REGISTER_BIT_1) ? TRUE : FALSE;
    charger_status->status_6.jeita_adc_hot = (data2[3] & PMIC_REGISTER_BIT_2) ? TRUE : FALSE;
    charger_status->status_6.jeita_adc_cold = (data2[3] & PMIC_REGISTER_BIT_3) ? TRUE : FALSE;
    charger_status->status_6.jeita_adc_tcold = (data2[3] & PMIC_REGISTER_BIT_4) ? TRUE : FALSE;
    charger_status->status_6.jeita_adc_tcold_afp = (data2[3] & PMIC_REGISTER_BIT_5) ? TRUE : FALSE;

    /*status 7 related*/
    charger_status->status_7.bat_temp_status_too_cold_afp = (data2[4] & PMIC_REGISTER_BIT_0) ? TRUE : FALSE;
    charger_status->status_7.bat_temp_status_too_hot_afp = (data2[4] & PMIC_REGISTER_BIT_1) ? TRUE : FALSE;
    charger_status->status_7.bat_temp_status_too_cold = (data2[4] & PMIC_REGISTER_BIT_2) ? TRUE : FALSE;
    charger_status->status_7.bat_temp_status_too_hot = (data2[4] & PMIC_REGISTER_BIT_3) ? TRUE : FALSE;
    charger_status->status_7.bat_temp_status_cold_soft = (data2[4] & PMIC_REGISTER_BIT_4) ? TRUE : FALSE;
    charger_status->status_7.bat_temp_status_hot_soft = (data2[4] & PMIC_REGISTER_BIT_5) ? TRUE : FALSE;

    /*status 8 related*/
    charger_status->status_8 = data2[5];

    return status;
    ;
}

PMSTATUS schg_chgr_get_chgr_status_r2_rev0(UINT8 pmic_index, PM_CHGR_CHGR_STATUS_R2_TYPE *charger_status)
{
    PMSTATUS status = STATUS_SUCCESS;

    PMIC_REGISTER_DATA_TYPE data[9] = {0};

    if (pmic_index >= PMIC_INDEX_INVALID || NULL == charger_status)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    // read registers 0x6 to 0xE
    status = PM_IN_MULTIBYTES(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_CHARGER_STATUS_ADDR, 9, data);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    /*status 1 related*/
    charger_status->status_1.battery_charger_status = data[0] & PMIO_SCPQ_P_CHGR_CHARGER_STATUS_BATTERY_CHARGER_STATUS_BMSK;
    charger_status->status_1.zero_charging_current = (data[4] & PMIO_SCPQ_P_CHGR_IBAT_STATUS_ZERO_CHARGE_CURRENT_BMSK) ? TRUE : FALSE;
    charger_status->status_1.icl_incr_req_for_prechg = (data[4] & PMIO_SCPQ_P_CHGR_IBAT_STATUS_ICL_INCR_REQ_FOR_PRECHG_BMSK) ? TRUE : FALSE;

    /*status 2 related*/
    charger_status->status_2.bat_ov = (data[0] & PMIO_SCPQ_P_CHGR_CHARGER_STATUS_CHARGER_ERROR_STATUS_BAT_OV_BMSK) ? TRUE : FALSE;
    charger_status->status_2.bat_sft_expire = (data[0] & PMIO_SCPQ_P_CHGR_CHARGER_STATUS_CHARGER_ERROR_STATUS_SFT_EXPIRE_BMSK) ? TRUE : FALSE;
    charger_status->status_2.bat_gt_full_on = (data[2] & PMIO_SCPQ_P_CHGR_VBAT_STATUS_VBATT_GT_FULL_ON_BMSK) ? TRUE : FALSE;
    charger_status->status_2.vbatt_gtet_inhibit = (data[2] & PMIO_SCPQ_P_CHGR_VBAT_STATUS_VBATT_GTET_INHIBIT_BMSK) ? TRUE : FALSE;
    charger_status->status_2.vbatt_ltet_recharge = (data[2] & PMIO_SCPQ_P_CHGR_VBAT_STATUS_VBATT_LTET_RECHARGE_BMSK) ? TRUE : FALSE;
    charger_status->status_2.drop_in_vbat_reference = (data[2] & PMIO_SCPQ_P_CHGR_VBAT_STATUS_DROP_IN_BATTERY_VOLTAGE_REFERENCE_BMSK) ? TRUE : FALSE;

    /*status 3 related*/
    charger_status->status_3 = data[5];

    /*status 4 related*/
    charger_status->status_4 = data[8];

    /*status 5 related*/
    charger_status->status_5.enable_fullon_mode = (data[1] & PMIO_SCPQ_P_CHGR_CHG_EN_STATUS_ENABLE_FULLON_MODE_BMSK) ? TRUE : FALSE;
    charger_status->status_5.enable_pre_charging = (data[1] & PMIO_SCPQ_P_CHGR_CHG_EN_STATUS_ENABLE_PRE_CHARGING_BMSK) ? TRUE : FALSE;
    charger_status->status_5.enable_trickle = (data[1] & PMIO_SCPQ_P_CHGR_CHG_EN_STATUS_ENABLE_TRICKLE_BMSK) ? TRUE : FALSE;
    charger_status->status_5.enable_chg_sensors = (data[1] & PMIO_SCPQ_P_CHGR_CHG_EN_STATUS_ENABLE_CHG_SENSORS_BMSK) ? TRUE : FALSE;
    charger_status->status_5.charging_enable = (data[1] & PMIO_SCPQ_P_CHGR_CHG_EN_STATUS_CHARGING_ENABLE_BMSK) ? TRUE : FALSE;
    charger_status->status_5.force_zero_charge_current = (data[4] & PMIO_SCPQ_P_CHGR_IBAT_STATUS_FORCE_ZERO_CHARGE_CURRENT_BMSK) ? TRUE : FALSE;
    charger_status->status_5.disable_charging = (data[1] & PMIO_SCPQ_P_CHGR_CHG_EN_STATUS_DISABLE_CHARGING_BMSK) ? TRUE : FALSE;
    charger_status->status_5.valid_input_power_source = (data[0] & PMIO_SCPQ_P_CHGR_CHARGER_STATUS_VALID_INPUT_POWER_SOURCE_BMSK) ? TRUE : FALSE;

    /*status 8 related*/
    charger_status->status_8 = (((data[2] & PMIO_SCPQ_P_CHGR_VBAT_STATUS_VBATT_LT_2V_BMSK) >> PMIO_SCPQ_P_CHGR_VBAT_STATUS_VBATT_LT_2V_SHFT) << 7) |
                                    (((data[3] & PMIO_SCPQ_P_CHGR_VBAT_STATUS2_PRE_FULLON_BMSK) >> PMIO_SCPQ_P_CHGR_VBAT_STATUS2_PRE_FULLON_SHFT) << 6) |
                                    (((data[3] & PMIO_SCPQ_P_CHGR_VBAT_STATUS2_TAPER_BMSK) >> PMIO_SCPQ_P_CHGR_VBAT_STATUS2_TAPER_SHFT) << 5) |
                                    (((data[3] & PMIO_SCPQ_P_CHGR_VBAT_STATUS2_PRE_OVRV_BMSK) >> PMIO_SCPQ_P_CHGR_VBAT_STATUS2_PRE_OVRV_SHFT) << 4) |
                                    (((data[3] & PMIO_SCPQ_P_CHGR_VBAT_STATUS2_PRE_INHIBIT_BMSK) >> PMIO_SCPQ_P_CHGR_VBAT_STATUS2_PRE_INHIBIT_SHFT) << 3) |
                                    (((data[4] & PMIO_SCPQ_P_CHGR_IBAT_STATUS_PRE_TERM_BMSK) >> PMIO_SCPQ_P_CHGR_IBAT_STATUS_PRE_TERM_SHFT) << 2) |
                                    (((data[2] & PMIO_SCPQ_P_CHGR_VBAT_STATUS_TAPER_REGION_BMSK) >> PMIO_SCPQ_P_CHGR_VBAT_STATUS_TAPER_REGION_SHFT) << 1) |
                                    (((data[2] & PMIO_SCPQ_P_CHGR_VBAT_STATUS_GF_BATT_OV_BMSK) >> PMIO_SCPQ_P_CHGR_VBAT_STATUS_GF_BATT_OV_SHFT) << 0);

    return status;
}

PMSTATUS schg_chgr_get_chgr_status_r2_rev1(UINT8 pmic_index, PM_CHGR_CHGR_STATUS_R2_TYPE *charger_status)
{
    PMSTATUS status = STATUS_SUCCESS;
    PM_CHGR_CHGR_STATUS_TYPE tmp_chgr_sts = {0};

    status = schg_chgr_get_chgr_status_rev1(pmic_index, &tmp_chgr_sts);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }
    /*status 1 related*/
    charger_status->status_1.battery_charger_status = tmp_chgr_sts.status_1.battery_charger_status;
    charger_status->status_1.zero_charging_current = tmp_chgr_sts.status_1.zero_charging_current;
    charger_status->status_1.icl_incr_req_for_prechg = tmp_chgr_sts.status_1.icl_incr_req_for_prechg;

    /*status 2 related*/
    charger_status->status_2.bat_ov = tmp_chgr_sts.status_2.bat_ov;
    charger_status->status_2.bat_sft_expire = tmp_chgr_sts.status_2.bat_sft_expire;
    charger_status->status_2.bat_gt_full_on = tmp_chgr_sts.status_2.bat_gt_full_on;
    charger_status->status_2.vbatt_gtet_inhibit = tmp_chgr_sts.status_2.vbatt_gtet_inhibit;
    charger_status->status_2.vbatt_ltet_recharge = tmp_chgr_sts.status_2.vbatt_ltet_recharge;
    charger_status->status_2.drop_in_vbat_reference = tmp_chgr_sts.status_2.drop_in_vbat_reference;

    /*status 3 related*/
    charger_status->status_3 = tmp_chgr_sts.status_3;

    /*status 4 related*/
    charger_status->status_4 = tmp_chgr_sts.status_4;

    /*status 5 related*/
    charger_status->status_5.enable_fullon_mode = tmp_chgr_sts.status_5.enable_fullon_mode;
    charger_status->status_5.enable_pre_charging = tmp_chgr_sts.status_5.enable_pre_charging;
    charger_status->status_5.enable_trickle = tmp_chgr_sts.status_5.enable_trickle;
    charger_status->status_5.enable_chg_sensors = tmp_chgr_sts.status_5.enable_chg_sensors;
    charger_status->status_5.charging_enable = tmp_chgr_sts.status_5.charging_enable;
    charger_status->status_5.force_zero_charge_current = tmp_chgr_sts.status_5.force_zero_charge_current;
    charger_status->status_5.disable_charging = tmp_chgr_sts.status_5.disable_charging;
    charger_status->status_5.valid_input_power_source = tmp_chgr_sts.status_5.valid_input_power_source;

    /*status 8 related*/
    charger_status->status_8 = tmp_chgr_sts.status_8;

    return status;
}

PMSTATUS schg_chgr_set_float_volt_rev1(UINT8 pmic_index, UINT32 float_volt_mv)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    if (float_volt_mv < FLOAT_VOLT_MIN)
    {
        data = 0x0;
    }
    else if (float_volt_mv >= gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX])
    {
        data = (gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX] - FLOAT_VOLT_MIN) / FLOAT_VOLT_STEP_CONSTANT;
    }
    else
    {
        /* Float voltage setting = 7.2V + (DATA x 20mV) */
        data = (float_volt_mv - FLOAT_VOLT_MIN) / FLOAT_VOLT_STEP_CONSTANT;
    }

    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_FLOAT_VOLTAGE_CFG, SCHG_CHGR_FLOAT_VOLTAGE_CFG_MASK, data);

    return status;
}

PMSTATUS schg_chgr_set_float_volt_rev0(UINT8 pmic_index, UINT32 float_volt_mv)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    if (float_volt_mv < FLOAT_VOLT_MIN_PM8150B)
    {
        data = 0x0;
    }
    else if (float_volt_mv >= gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX])
    {
        data = (gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX] - FLOAT_VOLT_MIN_PM8150B) / FLOAT_VOLT_STEP_CONSTANT_PM8150B;
    }
    else
    {
        /* Float voltage setting = 7.2V + (DATA x 10mV) */
        data = (float_volt_mv - FLOAT_VOLT_MIN_PM8150B) / FLOAT_VOLT_STEP_CONSTANT_PM8150B;
    }

    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_FLOAT_VOLTAGE_CFG, SCHG_CHGR_FLOAT_VOLTAGE_CFG_MASK, data);

    return status;
}

PMSTATUS schg_chgr_get_float_volt_rev0(UINT8 pmic_index, UINT32 *float_volt_mv)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID || float_volt_mv == NULL)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_BATTERY_CHARGER_STATUS_3, &data);

    if (status == STATUS_SUCCESS)
    {
        *float_volt_mv = FLOAT_VOLT_STEP_CONSTANT_PM8150B * data + FLOAT_VOLT_MIN_PM8150B;
    }

    return status;
}

PMSTATUS schg_chgr_set_float_volt_rev2(UINT8 pmic_index, UINT32 float_volt_mv)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    if (float_volt_mv < FLOAT_VOLT_MIN_PM8350B)
    {
        data = 0x0;
    }
    else if (float_volt_mv >= gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX])
    {
        data = (gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MAX] - FLOAT_VOLT_MIN_PM8350B) / FLOAT_VOLT_STEP_CONSTANT_PM8350B;
    }
    else
    {
        /* Float voltage setting = 3.6V + (DATA x 10mV) */
        data = (float_volt_mv - FLOAT_VOLT_MIN_PM8350B) / FLOAT_VOLT_STEP_CONSTANT_PM8350B;
    }

    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                            PMIO_SCPQ_P_CHGR_VFLT_CFG_ADDR,
                            PMIO_SCPQ_P_CHGR_VFLT_CFG_FLOAT_VOLTAGE_SETTING_BMSK,
                            data);

    return status;
}

PMSTATUS schg_chgr_get_float_volt_rev2(UINT8 pmic_index, UINT32 *float_volt_mv)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = 0;
    PMIC_REGISTER_DATA_TYPE cfg_2s = 0;

    if (pmic_index >= PMIC_INDEX_INVALID || float_volt_mv == NULL)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }
    status = PM_IN_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_BAT_2S_CHARGE_CFG_ADDR, PMIO_SCPQ_P_CHGR_BAT_2S_CHARGE_CFG_BAT_2S_CHARGE_CFG_BMSK, &cfg_2s);
    status |= PM_IN_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_VFLT_STATUS_ADDR, PMIO_SCPQ_P_CHGR_VFLT_STATUS_RMSK, &data);

    if (status == STATUS_SUCCESS)
    {
        *float_volt_mv = FLOAT_VOLT_STEP_CONSTANT_PM8350B * data + FLOAT_VOLT_MIN_PM8350B;
    }
    
    *float_volt_mv = cfg_2s ? (*float_volt_mv * 2) : *(float_volt_mv);
    
    return status;
}

PMSTATUS schg_chgr_set_jeita_fvcomp_cfg_rev1(UINT8 pmic_index, UINT8 fv_comp_hot, UINT8 fv_comp_cold)
{
    PMSTATUS status = STATUS_SUCCESS;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {

        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status |= PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_FVCOMP_HOT_CFG, SCHG_CHGR_JEITA_FVCOMP_HOT_CFG_MASK, fv_comp_hot);

    status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_FVCOMP_COLD_CFG, fv_comp_cold);

    return status;
}

PMSTATUS schg_chgr_set_jeita_cccomp_rev1(UINT8 pmic_index, UINT8 cccomp_hot, UINT8 cccomp_cold)
{
    PMSTATUS status = STATUS_SUCCESS;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status |= PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_CCCOMP_HOT_CFG, SCHG_CHGR_JEITA_CCCOMP_HOT_CFG_MASK, cccomp_hot);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status |= PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_CCCOMP_COLD_CFG, SCHG_CHGR_JEITA_CCCOMP_COLD_CFG_MASK, cccomp_cold);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    return status;
}

PMSTATUS schg_chgr_set_jeita_threshold_value_rev1(UINT8 pmic_index, PM_CHGR_JEITA_THRESHOLD_TYPE threshold_type, UINT16 jeita_threshold)
{
    PMSTATUS status = STATUS_SUCCESS;
    UINT8 jeita_threshold_lsb = jeita_threshold & 0xFF;
    UINT8 jeita_threshold_msb = jeita_threshold >> 8;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    switch (threshold_type)
    {
        case PM_SCHG_CHGR_JEITA_THRESHOLD_HOT:
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_HOT_THRESHOLD_MSB, jeita_threshold_msb);
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_HOT_THRESHOLD_LSB, jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_COLD:
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_COLD_THRESHOLD_MSB, jeita_threshold_msb);
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_COLD_THRESHOLD_LSB, jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_TOO_HOT:
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_THOT_THRESHOLD_MSB, jeita_threshold_msb);
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_THOT_THRESHOLD_LSB, jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_TOO_COLD:
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_TCOLD_THRESHOLD_MSB, jeita_threshold_msb);
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_TCOLD_THRESHOLD_LSB, jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_TOO_HOT_AFP:
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_THOT_AFP_THRESHOLD_MSB, jeita_threshold_msb);
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_THOT_AFP_THRESHOLD_LSB, jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_TOO_COLD_AFP:
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_TCOLD_AFP_THRESHOLD_MSB, jeita_threshold_msb);
            status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_TCOLD_AFP_THRESHOLD_LSB, jeita_threshold_lsb);
            break;

        default:
            return STATUS_ERROR_NOT_SUPPORTED;
            break;
    }

    return status;
}

PMSTATUS schg_chgr_get_jeita_threshold_value_rev1(UINT8 pmic_index, PM_CHGR_JEITA_THRESHOLD_TYPE threshold_type, UINT16 *jeita_threshold)
{
    PMSTATUS status = STATUS_SUCCESS;
    UINT8 jeita_threshold_lsb;
    UINT8 jeita_threshold_msb;

    if (pmic_index >= PMIC_INDEX_INVALID)
        return STATUS_ERROR_INVALID_ARGUMENT;

    switch (threshold_type)
    {
        case PM_SCHG_CHGR_JEITA_THRESHOLD_HOT:
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_HOT_THRESHOLD_MSB, &jeita_threshold_msb);
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_HOT_THRESHOLD_LSB, &jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_COLD:
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_COLD_THRESHOLD_MSB, &jeita_threshold_msb);
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_COLD_THRESHOLD_LSB, &jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_TOO_HOT:
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_THOT_THRESHOLD_MSB, &jeita_threshold_msb);
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_THOT_THRESHOLD_LSB, &jeita_threshold_lsb);
            break;

        case PM_SCHG_CHGR_JEITA_THRESHOLD_TOO_COLD:
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_TCOLD_THRESHOLD_MSB, &jeita_threshold_msb);
            status |= PM_IN(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_TCOLD_THRESHOLD_LSB, &jeita_threshold_lsb);
            break;

        default:
            return STATUS_ERROR_NOT_SUPPORTED;
    }
    if (status != STATUS_SUCCESS){
        return status;
    }
	*jeita_threshold  = (UINT16)jeita_threshold_msb;
	*jeita_threshold = ((*jeita_threshold) << 8) | (UINT16)jeita_threshold_lsb;

    return status;
}

PMSTATUS schg_chgr_set_jeita_en_cfg_rev1(UINT8 pmic_index, PM_CHGR_JEITA_CFG_TYPE jeita_cfg_type, BOOLEAN enable)
{
    PMSTATUS status = STATUS_SUCCESS;

    if (pmic_index >= PMIC_INDEX_INVALID || jeita_cfg_type >= PM_SCHG_CHGR_JEITA_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }
    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_JEITA_EN_CFG, (1 << jeita_cfg_type), ((enable) ? 0xFF : 0x00));

    return status;
}

PMSTATUS schg_chgr_get_batt_temp_status_rev1(UINT8 pmic_index, PM_CHGR_BATT_TEMP_STATUS_TYPE* batt_temp_status)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID || batt_temp_status == NULL)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_IN_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                            SCHG_CHGR_BATTERY_CHARGER_STATUS_7,
                            SCHG_CHGR_BATTERY_CHARGER_STATUS_7_MASK,
                            &data);
    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    batt_temp_status->bat_temp_status_too_cold_afp = (data & SCHG_CHGR_BAT_TEMP_STATUS_BAT_TEMP_AFP_COLD_MASK) ? TRUE : FALSE;
    batt_temp_status->bat_temp_status_too_hot_afp = (data & SCHG_CHGR_BAT_TEMP_STATUS_BAT_TEMP_AFP_HOT_MASK) ? TRUE : FALSE;
    batt_temp_status->bat_temp_status_too_cold = (data & SCHG_CHGR_BAT_TEMP_STATUS_BAT_TEMP_TOO_COLD_MASK) ? TRUE : FALSE;
    batt_temp_status->bat_temp_status_too_hot = (data & SCHG_CHGR_BAT_TEMP_STATUS_BAT_TEMP_TOO_HOT_MASK) ? TRUE : FALSE;
    batt_temp_status->bat_temp_status_cold_soft = (data & SCHG_CHGR_BAT_TEMP_STATUS_BAT_TEMP_COOL_MASK) ? TRUE : FALSE;
    batt_temp_status->bat_temp_status_hot_soft = (data & SCHG_CHGR_BAT_TEMP_STATUS_BAT_TEMP_WARM_MASK) ? TRUE : FALSE;

    return status;
}

PMSTATUS schg_chgr_set_fast_charge_safety_timer_cfg_rev1(UINT8 pmic_index, PM_CHGR_FAST_CHARGE_SAFETY_TIMER_TYPE set_value)
{
    PMSTATUS status = STATUS_SUCCESS;
    UINT8 enable = 0;
    if (pmic_index >= PMIC_INDEX_INVALID || set_value > PM_SCHG_CHGR_FC_TMOUT_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }
    if (set_value == PM_SCHG_CHGR_FC_TMOUT_INVALID)
    {
        //if it is invalid safety timer, disable safety timer
        status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_ENABLE_CFG, SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_ENABLE_CFG_MASK, enable);
    }
    else
    {
        enable = 1;
        status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_ENABLE_CFG, SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_ENABLE_CFG_MASK, enable);
        status |= PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_CFG, SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_CFG_MASK, set_value);
    }

    return status;
}

PMSTATUS schg_chgr_set_fast_charge_safety_timer_cfg_rev2(UINT8 pmic_index, PM_CHGR_FAST_CHARGE_SAFETY_TIMER_TYPE set_value)
{
    PMSTATUS status = STATUS_SUCCESS;
    UINT8 enable = 0;
    if (pmic_index >= PMIC_INDEX_INVALID || set_value > PM_SCHG_CHGR_FC_TMOUT_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }
    if (set_value == PM_SCHG_CHGR_FC_TMOUT_INVALID)
    {
        //if it is invalid safety timer, disable safety timer
        status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                                PMIO_SCPQ_P_CHGR_CHG_SAFETY_TIMER_CFG_ADDR,
                                PMIO_SCPQ_P_CHGR_CHG_SAFETY_TIMER_CFG_FAST_CHARGE_SAFETY_TIMER_EN_BMSK,
                                enable);
    }
    else
    {
        enable = (1 << PMIO_SCPQ_P_CHGR_CHG_SAFETY_TIMER_CFG_FAST_CHARGE_SAFETY_TIMER_EN_SHFT);
        status |= PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                        PMIO_SCPQ_P_CHGR_CHG_SAFETY_TIMER_CFG_ADDR, 
                        PMIO_SCPQ_P_CHGR_CHG_SAFETY_TIMER_CFG_FAST_CHARGE_SAFETY_TIMER_EN_BMSK | PMIO_SCPQ_P_CHGR_CHG_SAFETY_TIMER_CFG_FAST_CHARGE_SAFETY_TIMER_BMSK,
                        enable | set_value);
    }

    return status;
}

PMSTATUS schg_chgr_set_chgr_cfg2_rev1(UINT8 pmic_index, PM_CHGR_CHGR_CFG2_TYPE type, BOOLEAN set_value)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = set_value ? 0xFF : 0;
    PMIC_REGISTER_MASK_TYPE mask = 1 << type;

    if (pmic_index >= PMIC_INDEX_INVALID || type >= PM_SCHG_CHGR_CFG2_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_CHGR_CFG2, mask, data);

    return status;
}

PMSTATUS schg_chgr_set_chgr_cfg2_rev2(UINT8 pmic_index, PM_CHGR_CHGR_CFG2_TYPE type, BOOLEAN set_value)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE data = set_value ? 0xFF : 0x00;

    if (pmic_index >= PMIC_INDEX_INVALID || type >= PM_SCHG_CHGR_CFG2_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    // PM_SCHG_CHGR_BAT_OV_ECC  bit moved to batif
    // PM_SCHG_CHGR_EN_FAVOR_IN bit moved to misc
    // PM_SCHG_CHGR_CHG_EN_POLARITY
    // PM_SCHG_CHGR_CHG_EN_SRC
    if (type == PM_SCHG_CHGR_CHARGER_INHIBIT)
    {
        status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                                PMIO_SCPQ_P_CHGR_CHG_INHIBIT_CFG_ADDR,
                                PMIO_SCPQ_P_CHGR_CHG_INHIBIT_CFG_CHARGER_INHIBIT_BMSK,
                                data);
    }
    else if (type == PM_SCHG_CHGR_SOC_BASED_RECHG)
    {
        status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                                PMIO_SCPQ_P_CHGR_RECHG_CFG_ADDR,
                                PMIO_SCPQ_P_CHGR_RECHG_CFG_SOC_BASED_RECHG_BMSK,
                                data);
    }
    else if (type == PM_SCHG_CHGR_AUTO_RECHG)
    {
        status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                                PMIO_SCPQ_P_CHGR_RECHG_CFG_ADDR,
                                PMIO_SCPQ_P_CHGR_RECHG_CFG_AUTO_RECHG_BMSK,
                                data);
    }
    else if (type == PM_SCHG_CHGR_I_TERM)
    {
        status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId,
                                PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_ADDR,
                                PMIO_SCPQ_P_CHGR_CHG_TERM_CFG_I_TERM_BMSK,
                                data);
    }
    else
    {
        return STATUS_ERROR_NOT_SUPPORTED;
    }

    return status;
}

PMSTATUS schg_chgr_set_charge_inhibit_threshold_cfg_rev1(UINT8 pmic_index, PM_CHGR_CHARGE_INHIBIT_THRESHOLD_TYPE set_value)
{
    PMSTATUS status = STATUS_SUCCESS;

    if (pmic_index >= PMIC_INDEX_INVALID || set_value >= PM_SCHG_CHGR_INHIBIT_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, SCHG_CHGR_CHARGE_INHIBIT_THRESHOLD_CFG, SCHG_CHGR_CHARGE_INHIBIT_THRESHOLD_CFG_MASK, set_value);

    return status;
}

PMSTATUS schg_chgr_set_charge_inhibit_threshold_cfg_rev2(UINT8 pmic_index, PM_CHGR_CHARGE_INHIBIT_THRESHOLD_TYPE set_value)
{
    PMSTATUS status = STATUS_SUCCESS;
    UINT16 data[] = {0x202, 0x404, 0x808, 0xc0b};

    if (pmic_index >= PMIC_INDEX_INVALID || set_value >= PM_SCHG_CHGR_INHIBIT_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    
}

    // Inhibit threshold value is now adc based value
    // 1 LSB = 194.6uV.
    // data[] is pre calculated array
    status = PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_CHG_INHIBIT_THD_MSB_ADDR, data[set_value] >> 8);
    status |= PM_OUT(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_CHG_INHIBIT_THD_MSB_ADDR, data[set_value] & 0xFF);

    return status;
}

PMSTATUS schg_chgr_set_dig_term_current(UINT8 pmic_index, INT32 current) {
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_ADDRESS_TYPE reg = PMIO_SCPQ_P_CHGR_ADC_ITERM_UP_THD_MSB_ADDR;
    PMIC_REGISTER_DATA_TYPE data[2] = {0xFD, 0x71}; //Default 200mA
    UINT8 NumBytes = 2;
    INT16 reg_data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    reg_data = (INT16)((current * INT16_MAX_VALUE) / DIGITAL_TERMINATON_CURRENT_MAX);
    data[0] = (UINT8)((reg_data & 0xFF00) >> 8);
    data[1] = (UINT8)(reg_data & 0x00FF);
    status = (PMSTATUS)PM_OUT_MULTIBYTES(gSchgChgrConfig[pmic_index].PeripheralSlaveId, reg, NumBytes, data);
    
    return status;
}

PMSTATUS schg_chgr_set_dig_term_current_rev1(UINT8 pmic_index, INT32 current) 
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_ADDRESS_TYPE reg = SCHG_CHGR_ADC_ITERM_UP_THD_MSB_ADDR;
    PMIC_REGISTER_DATA_TYPE data[2] = {0};
    UINT8 NumBytes = 2;
    INT16 reg_data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    reg_data = (INT16)((current * INT16_MAX_VALUE) / DIGITAL_TERMINATON_CURRENT_MAX);
    data[0] = (UINT8)((reg_data & 0xFF00) >> 8);
    data[1] = (UINT8)(reg_data & 0x00FF);
    status = (PMSTATUS)PM_OUT_MULTIBYTES(gSchgChgrConfig[pmic_index].PeripheralSlaveId, reg, NumBytes, data);
    
    return status;
}

PMSTATUS schg_chgr_set_dig_term_current_rev2(UINT8 pmic_index, INT32 current)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_ADDRESS_TYPE reg = SCHG_CHGR_ADC_ITERM_UP_THD_MSB_ADDR;
    PMIC_REGISTER_DATA_TYPE data[2] = {0};
    UINT8 NumBytes = 2;
    INT16 reg_data = 0;

    if (pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    reg_data = (INT16)((current * INT16_MAX_VALUE) / DIGITAL_TERMINATON_CURRENT_MAX_PMI632);
    data[0] = (UINT8)((reg_data & 0xFF00) >> 8);
    data[1] = (UINT8)(reg_data & 0x00FF);
    status = (PMSTATUS)PM_OUT_MULTIBYTES(gSchgChgrConfig[pmic_index].PeripheralSlaveId, reg, NumBytes, data);

    return status;
}

PMSTATUS schg_smb_en_passtrough_control_rev2(UINT8 pmic_index, BOOLEAN enable, INT32 cell)
{
    PMSTATUS status = STATUS_SUCCESS;
    PMIC_REGISTER_DATA_TYPE mask, val;

    mask = USE_VBATT_OV_ADC_BMSK | USE_IBATT_OC_ADC;
    val = enable ? (USE_VBATT_OV_ADC_BMSK | USE_IBATT_OC_ADC): 0;
    status |= PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_DIRECT_CHARGE_BATT_HW_DIS_CFG_ADDR,
                          mask, val);
   
    mask  = USE_JEITA_HOT_ADC | USE_JEITA_COLD_ADC | USE_CONN_ADC | USE_SMB_ADC;
    val = enable ? (USE_JEITA_HOT_ADC | USE_JEITA_COLD_ADC | USE_CONN_ADC | USE_SMB_ADC) : 0; 
    status |= PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_DIRECT_CHARGE_TEMP_HW_DIS_CFG_ADDR,
                          mask, val);

    //TODO: we can use cell to configure 1S/2S
    status = PM_OUT_MASK(gSchgChgrConfig[pmic_index].PeripheralSlaveId, PMIO_SCPQ_P_CHGR_DIRECT_CHARGE_CTRL_ADDR,
                         PMIO_SCPQ_P_CHGR_EN_DIRECT_CHARGE_SMB_EN_BMSK | EN_DIRECT_CHARGE_1S_BMSK,
                         enable ? (PMIO_SCPQ_P_CHGR_EN_DIRECT_CHARGE_SMB_EN_BMSK | EN_DIRECT_CHARGE_1S_BMSK) : 0);


    return status;
}

PMSTATUS schg_chgr_get_float_volt_constants(UINT8 pmic_index, UINT32* float_volt_min, UINT32* float_volt_step)
{
    if (float_volt_min == NULL || float_volt_step == NULL || pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    *float_volt_min = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_MIN];
    *float_volt_step = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__FLOAT_VOLT_STEP];
    return STATUS_SUCCESS;
}

PMSTATUS schg_chgr_get_term_current_constants(UINT8 pmic_index, UINT32* iterm_min, UINT32* iterm_step)
{
    if (iterm_min == NULL || iterm_step == NULL || pmic_index >= PMIC_INDEX_INVALID)
    {
        return STATUS_ERROR_INVALID_ARGUMENT;
    }

    *iterm_min = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_MIN];
    *iterm_step = gSchgChgrConfig[pmic_index].HwRevConfig[PM_SCHG_CHGR_HWREV_CONFIG__TERM_CURRENT_STEP];
    return STATUS_SUCCESS;
}

