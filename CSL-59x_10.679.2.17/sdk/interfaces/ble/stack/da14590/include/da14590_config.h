


#if ALTER_DEV >= 0x20
#define CFG_CON                 2
#else
#define CFG_CON                 8
#endif
#define BLE_CONNECTION_MAX_USER CFG_CON


#ifndef HOST_LIB

#define DA14690                 1

//include CH2
#define F_CH2
//include 2M
#define F_2M

#define CFG_NVDS
#define CFG_EMB
#define CFG_HOST
#define nCFG_APP
#define CFG_GTL
#define CFG_BLE
#define CFG_HCI_UART
#define CFG_EXT_DB
#define nCFG_DBG_MEM
#define CFG_DBG_FLASH
#define nCFG_DBG_NVDS
#define nCFG_DBG_STACK_PROF
#define CFG_RF_RIPPLE
#define nCFG_PERIPHERAL
#define CFG_ALLROLES            1
#define CFG_SECURITY_ON         1
#define CFG_ATTC
#define CFG_ATTS
#define nCFG_PRF
#define CFG_NB_PRF              10
#define nCFG_DBG
#define DA_DBG_TASK

#define CFG_WLAN_COEX
//#define CFG_WLAN_COEX_TEST
//#define CFG_BLE_TESTER

#define CFG_H4TL                1

#define CFG_APP_AFMN
#define CFG_APP_GFMDN

//#define CFG_PRF_ACCEL         1
#define POWER_OFF_SLEEP
#ifndef ALTER_DEV
#define RIPPLE_ID               49
#else
#define RIPPLE_ID               19
#endif
#define CFG_BLECORE_11
#define CFG_SLEEP

#define CFG_DEEP_SLEEP
#define __NO_EMBEDDED_ASM
#define nCFG_APP_SEC
#define CFG_CHNL_ASSESS
#define CFG_DBG_NVDS
#define CFG_DBG_MEM

#define nCFG_PRF_BASC           1
#define nCFG_PRF_BASS           1
#define nCFG_PRF_DISC           1
#define nCFG_PRF_DISS           1

#define nCFG_PRF_ANPC           1
#define nCFG_PRF_ANPS           1
#define nCFG_PRF_BLPC           1
#define nCFG_PRF_BLPS           1
#define nCFG_PRF_CSCPC          1
#define nCFG_PRF_CSCPS          1
#define nCFG_PRF_FMPL           1
#define nCFG_PRF_FMPT           1
#define nCFG_PRF_GLPC           1
#define nCFG_PRF_GLPS           1
#define nCFG_PRF_HOGPD          1
#define nCFG_PRF_HOGPBH         1
#define nCFG_PRF_HOGPRH         1
#define nCFG_PRF_HRPC           1
#define nCFG_PRF_HRPS           1
#define nCFG_PRF_HTPT           1
#define nCFG_PRF_HTPC           1
#define nCFG_PRF_PASPC          1
#define nCFG_PRF_PASPS          1
#define nCFG_PRF_PXPM           1
#define nCFG_PRF_PXPR           1
#define nCFG_PRF_RSCPC          1
#define nCFG_PRF_RSCPS          1
#define nCFG_PRF_SCPPC          1
#define nCFG_PRF_SCPPS          1
#define nCFG_PRF_TIPC           1
#define nCFG_PRF_TIPS           1
#define nCFG_PRF_CPPC           1
#define nCFG_PRF_CPPS           1

#define nSEC_TEST               1
//#define RAM_BUILD


#endif  // HOST_BUILD
