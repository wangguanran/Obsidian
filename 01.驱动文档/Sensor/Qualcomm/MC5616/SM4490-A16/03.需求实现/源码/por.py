#==============================================================================
# POR Config Script
#
# Copyright (c) 2022-2023 Qualcomm Technologies Inc.
# All Rights Reserved
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#==============================================================================

include_sensor_vendor_libs = []
include_oem_libs = []
include_qsh_libs = []
exclude_libs = []

def exists(env):
   return env.Detect('por')

def generate(env):

    #disabling registry for presil
    #env.AddUsesFlags(['SNS_DISABLE_REGISTRY'])
    #env.Append(CPPDEFINES = ['SNS_DISABLE_REGISTRY'])
    
    if 'SSC_TARGET_X86' not in env['CPPDEFINES']:
        # POR sensors list
        include_sensor_vendor_libs.extend(['sns_sc7a20'])
        env.Replace(SSC_INCLUDE_SENS_VEND_LIBS=include_sensor_vendor_libs)

        # TODO : Disable close to WAIPIO CS (Enable Registry Debug)
        env.AddUsesFlags(['USES_ENABLE_REG_DEBUG'])

        # OEM sensors list
        include_oem_libs.extend([])
        env.Replace(SSC_INCLUDE_OEM_LIBS=include_oem_libs)

        # QSH sensors list
        if env.PathExists("${BUILD_ROOT}/qsh") and env.GetUsesFlag('USES_QSH_SENSORS'):
            include_qsh_libs.extend(['qsh_ble',
                                     'qsh_ble_test',
                                     'qsh_geofence',
                                     'qsh_location',
                                     'qsh_location_common',
                                     'qsh_loc_test',
                                     'qsh_wifi',
                                     'qsh_wwan',
                                     'qsh_oem1'])
            #qsh audio sensors
            include_qsh_libs.extend(['qsh_audio_utils',
                                     'qsh_audio_test',
                                     'qsh_audio_event',
                                     'qsh_audio_context',
                                     'qsh_audio_upd_proxy',
                                     'qsh_audio_data'])
            #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_QSH_AUDIO'])
            if env.GetUsesFlag('USES_UART_ISLAND') and 'USES_2_MB_LPI' in env:
                env.AddUsesFlags(['SNS_ISLAND_INCLUDE_QSH_BLE'])

            env.Replace(SSC_INCLUDE_QSH_LIBS=include_qsh_libs)
            env.Append(CPPDEFINES = ['SSC_QSH_SENSORS_LIST'])
        else:
            env.Replace(SSC_INCLUDE_QSH_LIBS={})

        env.Replace(SSC_EXCLUDE_LIBS=exclude_libs)
        env.Append(CFLAGS = '-Wno-unused-parameter')
        env.Append(CFLAGS = '-fno-jump-tables')
        env.Append(CFLAGS='-DSSC_BASE=LPASS_BASE')

        env.AddUsesFlags(['USES_MULTIPLE_SENSORS'])
        env.AddUsesFlags(['LSM6DST_ENABLE_DUAL_SENSOR'])

        # Referenced by SSC and SDC
        #env.Append(CPPDEFINES = ['SSC_TARGET_NO_CCD'])

        if ('SSC_BUILD_TAGS' in env and env.IsKeyEnable(env['SSC_BUILD_TAGS']) is True) or env.IsKeyEnable(['SSC_SHARED_LIBS']) is True:
            env.Append(CPPDEFINES = ['SSC_TARGET_HEXAGON_CORE_QDSP6_2_0'])
            env.Append(CPPDEFINES = ['SSC_TARGET_HEXAGON_CORE_QDSP6_3_0'])
            env.Append(CPPDEFINES = ['SSC_TARGET_HEXAGON_CORE_QDSP6_4_0'])
            env.Append(CPPDEFINES = ['SSC_TARGET_PRAM_AVAILABLE'])
            #env.Append(CPPDEFINES = ['SSC_TARGET_SDC_AVAILABLE'])
            env.Append(CPPDEFINES = ['SSC_TARGET_IPCC_AVAILABLE'])
            env.Append(CPPDEFINES = ['SSC_TARGET_GPIO_2_0'])
            env.Append(CPPDEFINES = ['SNS_GDSC_BLOCK_ENABLE'])
            #env.Append(CPPDEFINES = ['SNS_SDC_CLOCK_VOTING_ENABLE'])
            #env.Append(CPPDEFINES = ['IBI_REQUIRES_POWER_ON']) #to be removed once buses team support IBI with power off

            #env.Append(CPPDEFINES = ['SNS_QSH_FUSE_ENABLE'])
            if 'USES_CORE_MEMORY_OPT_CHRE' not in env:
                env.Append(CPPDEFINES = ['SNS_ISLAND_INCLUDE_DIAG'])

            #env.AddUsesFlags(['SNS_ENABLE_DAE'])
            #PEND: Remove following line once CoreBSP resolves compiler warnings
            #env.Append(CPPDEFINES = ['SNS_USE_LOCAL_CLK_SRC'])
            env.Append(CPPDEFINES = ['SNS_ISLAND_INCLUDE_SPI'])
            env.Append(CPPDEFINES = ['SNS_ISLAND_INCLUDE_I2C'])
            if env.GetUsesFlag('USES_UART_ISLAND') and 'USES_2_MB_LPI' in env:
                env.Append(CPPDEFINES = ['SNS_INCLUDE_UART'])
            env.Append(CPPDEFINES = ['SNS_BANDWIDTH_VOTING_ENABLED'])
            
            if 'USES_FEATURE_MINIDUMP' in env:
                env.Append(CPPDEFINES = ['SSC_USERPD_MINIDUMP_ENABLE'])

            # Dynamic libraries support
            env.AddUsesFlags(['SNS_SHARELIB_BUILDER'])
            #env.AddUsesFlags(['SNS_DYNLIB_GEOMAG_RV'])
            env.Append(CPPDEFINES = ['SNS_DYNLIB_NUM_OF_ISLAND_PAGES = 12']) # No. of pages required for island memory for the driver
            env.Append(CPPDEFINES = ['LSM6DST_LOG_VERBOSE_DEFAULT = 0']) # Disabling driver logging by set value to 0
            env.AddUsesFlags(['SNS_DYNLIB_LSM6DST'])
            env.AddUsesFlags(['SNS_DYNLIB_LOCAL_HEAPMGR'])
            
            

            if 'QDSS_TRACER_SWE' in env:
                env.Append(CPPDEFINES = ['SNS_LOCAL_TRACER_IDS'])
                env.SWEBuilder(['${BUILDPATH}/sns_tracer_event_ids.h'],None)
                env.Append(CPPPATH = ['${BUILD_ROOT}/ssc/build/${BUILDPATH}'])

    #if env.IsBuildInternal():
    #  env.AddUsesFlags(['SNS_ENABLE_QSOCKET_TEST_CLIENT'])

    # SDC/CCD configuration
    #env.Append(CPPDEFINES = ['DAE_855_L2VIC_MAP'])
    #env.Append(CPPDEFINES = ['SDC_WDOG_IRQ=115'])
    #env.Append(CPPDEFINES = ['SNS_CCD_IRQS=116,117,118,119,120,121,122,123,76,149,150,151'])
    #env.AddUsesFlags(['CCD_4'])
    #if 'USES_SDC_LOADER' in env:
    #    env.Append(CPPDEFINES = ['CORE_SDC_LOADER'])

    # Sensor Utilities version
    env.Append(CPPDEFINES = ['SSC_ISLAND_3_0'])
    env.Append(CPPDEFINES = ['SSC_PWR_SLEEP_MGR_4_0'])
    env.Append(CPPDEFINES = ['SSC_TIMER_1_0'])
    env.AddUsesFlags(['SNS_QURT_HAS_SET_STACK_SIZE2'])

    # Enable F3s in Island Mode
    env.AddUsesFlags(['USES_SNS_ENABLE_ISLAND_F3'])

    # Control new compiler option
    env.AddUsesFlags(['USES_NO_FRAME_ADDRESS'])

    # Diag disable flags.
    #env.Append(CPPDEFINES = ['SNS_PRINTF_DISABLED'])
    #env.Append(CPPDEFINES = ['SNS_LOG_DISABLED'])

    if 'USES_1P5_MB_LPI' in env:

        # 0. Framework      
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_CM'])

        # 1. Island drivers      
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_LSM6DST'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_LPS22HX'])
      
        # 2. Calibration
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GYRO_CAL'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_MAG_CAL'])      
      
        # 3. Algorithms group 1
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_AMD'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_PEDOMETER'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_SMD'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_PSMD'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_TILT_TO_WAKE'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_DEVICE_ORIENT'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_TILT'])
    
        # 4. Algorithm group 2
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GRM'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GRAVITY'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GAME_RV'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_FMV'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_ROTV'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GEOMAG_RV'])
          
        # 5. Algorithm group 3
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_THRESHOLD'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_CMC'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_DISTANCE_BOUND'])
        
        # 6. Algorithms group 4
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_RMD'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_BASIC_GESTURES'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_BRING_TO_EAR'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_FACING'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_MULTISHAKE'])
        #env.AddUsesFlags(['SNS_DPC_ISLAND_INCLUDE'])
        
        # 7. Test
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_DA_TEST'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_AONT'])
        
    else:
        #Island feature set for 2MB LPI availibility.
        # 0. Framework
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_CM'])

        # 1. Island drivers
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_LSM6DST'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_AK0991X'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_TMD2725'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_LPS22HX'])

        #include test sensor in island for pre-sil validations
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_TEST_SENSOR'])

        # 2. Calibration
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GYRO_CAL'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_MAG_CAL'])

        # 3. Algorithms group 1
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_AMD'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_TILT'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_PEDOMETER'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_SMD'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_PSMD'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_TILT_TO_WAKE'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_DEVICE_ORIENT'])

        # 4. Algorithm group 2
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GRM'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GRAVITY'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GAME_RV'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_FMV'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_ROTV'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_GEOMAG_RV'])

        # 5. Algorithm group 3
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_THRESHOLD'])
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_CMC'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_AR'])  
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_ED']) 
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_DISTANCE_BOUND'])

        # 6. Algorithms group 4
        #env.AddUsesFlags(['SNS_ISLAND_INCLUDE_RMD'])
        #env.AddUsesFlags(['SNS_DPC_ISLAND_INCLUDE'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_OEM1'])

        # 7. Test
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_DA_TEST'])
        env.AddUsesFlags(['SNS_ISLAND_INCLUDE_AONT'])

        # 8. Enable crash
        #env.Append(CPPDEFINES = ['USES_DRIVER_CRASH_ON_INVALID_STATE'])
        
        #9. Enable online playback 
        env.AddUsesFlags(['USES_ONLINE_PLAYBACK'])
        
        #10.Use ssc island pool if online playback is enable
        if 'USES_ONLINE_PLAYBACK' in env:
          #env.Append(CPPDEFINES = ['SNS_PLAYBACK_ISLAND_STATIC_BUFFER'])
          env.Append(CPPDEFINES = ['SNS_USE_SSC_ISLAND_POOL'])