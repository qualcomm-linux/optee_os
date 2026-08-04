#ifndef QFPROM_TARGET_NORD_H
#define QFPROM_TARGET_NORD_H

/*===========================================================================

                        QFPROM  Driver Header  File

DESCRIPTION
 Contains target specific defintions and APIs to be used to read and write
 qfprom values for sec ctrl 3.0 .

INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

/*===========================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/31/24   ask    Initial version

============================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qfprom.h"

/*===========================================================================

                      PUBLIC DATA DECLARATIONS

===========================================================================*/

// Start of TMT add for compilation issues
// from com_dtypes.h

#if (! defined T_WINNT) && (! defined __GNUC__)
    /* Non WinNT Targets */

    #if defined(__ARMCC_VERSION)
      #define ALIGN(__value) __align(__value)
    #else
      #ifndef FEATURE_WINCE
        #define ALIGN(__value) __attribute__((__aligned__(__value)))
      #else /* FEATURE_WINCE */
        #define ALIGN(__value)
      #endif /* FEATURE_WINCE */
    #endif /* defined (__GNUC__) */   
#else /* T_WINNT || TARGET_OS_SOLARIS || __GNUC__ */

    /* WINNT or SOLARIS based targets*/
    #if defined (__GNUC__) || defined (__GNUG__)
      #define ALIGN(__value) __attribute__((aligned(__value)))
    #else
      #define ALIGN(__value)
    #endif   
#endif /* T_WINNT */

// End of TMT add for compilation issues


typedef enum
{
  QFPROM_FEC_NONE = 0,
  QFPROM_FEC_15_10,
  QFPROM_FEC_63_56,
  QFPROM_FEC_62_56,

  /* Add above this */
  QFPROM_MAX_FEC
}QFPROM_FEC_SCHEME;

typedef enum
{
  QFPROM_CONV_FORMAT_RAW_TO_CORR = 0,
  QFPROM_CONV_FORMAT_CORR_TO_RAW = 1,
}QFPROM_CONV_FORMAT;

typedef enum
{
  QFPROM_ROW_LSB = 0,
  QFPROM_ROW_MSB,
}QFPROM_ROW;

/* Invalid entry */
#define QFPROM_INVALID_ENTRY 0xFFFFFFFF
            
/* Raw to Corrected Address region offset  */
#define QFPROM_RAW_TO_CORRECTED_ADDRESS_OFFSET   (0x2000)

/* Macro for Read/write permission of corrected address */
#define QFPROM_READ_PERM_CORRECTED_LSB_ADDR       HWIO_REMAPPED_QFPROM_CORR_READ_PERMISSIONS_LSB_ADDR
#define QFPROM_READ_PERM_CORRECTED_MSB_ADDR       HWIO_REMAPPED_QFPROM_CORR_READ_PERMISSIONS_MSB_ADDR
#define QFPROM_WRITE_PERM_CORRECTED_LSB_ADDR      HWIO_REMAPPED_QFPROM_CORR_WRITE_PERMISSIONS_LSB_ADDR
#define QFPROM_WRITE_PERM_CORRECTED_MSB_ADDR      HWIO_REMAPPED_QFPROM_CORR_WRITE_PERMISSIONS_MSB_ADDR

#define QFPROM_FEC_EN_CORRECTED_LSB_ADDR          HWIO_REMAPPED_QFPROM_CORR_FUSE_REDUNDANCY_ENABLE_LSB_ADDR
#define QFPROM_FEC_EN_CORRECTED_MSB_ADDR          HWIO_REMAPPED_QFPROM_CORR_FUSE_REDUNDANCY_ENABLE_MSB_ADDR
/*---------------------------------------------------------------------------
  QFPROM REGIONS 
---------------------------------------------------------------------------*/
typedef enum
{
  QFPROM_TME_PCD_REGION = 0,                   /*  0 */
  QFPROM_TME_PRIVATE_REGION,                   /*  1 */
  QFPROM_TME_LCS_REGION,                       /*  2 */
  QFPROM_MRC_REGION,                           /*  3 */
  QFPROM_QC_SECURITY_POLICY_REGION,            /*  4 */
  QFPROM_OEM_SECURITY_POLICY_REGION,           /*  5 */
  QFPROM_WRITE_PERMISSIONS_REGION,             /*  6 */
  QFPROM_READ_PERMISSIONS_REGION,              /*  7 */
  QFPROM_FUSE_REDUNDANCY_ENABLE_REGION,        /*  8 */
  QFPROM_DEBUG_DISABLE_REGION,                 /*  9 */
  QFPROM_DEBUG_TEST_REENABLE_REGION,           /* 10 */
  QFPROM_PTE_REGION,                           /* 11 */
  QFPROM_SECURITY_VERSIONING_REGION,           /* 12 */
  QFPROM_QC_CONFIG_REGION,                     /* 13 */
  QFPROM_OEM_CONFIG_REGION,                    /* 14 */
  QFPROM_FEATURE_CONFIG_REGION,                /* 15 */
  QFPROM_FEATURE_CONFIG_OVERRIDE_REGION,       /* 16 */
  QFPROM_ANTI_ROLLBACK_REGION,                 /* 17 */
  QFPROM_QC_ECC_REGION,                        /* 18 */
  QFPROM_M3_CERTIFICATE_REGION,                /* 19 */
  QFPROM_QC_SPARE_0_REGION,                    /* 20 */
  QFPROM_QC_SPARE_1_REGION,                    /* 21 */
  QFPROM_QC_SPARE_2_REGION,                    /* 22 */
  QFPROM_QC_SPARE_3_REGION,                    /* 23 */
  QFPROM_TME_SEQUENCER_RAM_PATCH_HASH_REGION,  /* 24 */
  QFPROM_TME_SEQUENCER_ROM_PATCH_REGION,       /* 25 */
  QFPROM_MRC_HASH_REGION,                      /* 26 */
  QFPROM_OEM_PRODUCT_SEED_REGION,              /* 27 */
  QFPROM_SOC_ROM_PATCH_REGION,                 /* 28 */
  QFPROM_TME_CPU_ROM_PATCH_REGION,             /* 29 */
  QFPROM_CALIBRATION_REGION,                   /* 30 */
  QFPROM_MEMORY_REDUN_REGION,                  /* 31 */
  QFPROM_MEMORY_ACC_REGION,                    /* 32 */
  QFPROM_OEM_SPARE_0_REGION,                   /* 33 */
  QFPROM_OEM_SPARE_1_REGION,                   /* 34 */
  QFPROM_OEM_SPARE_2_REGION,                   /* 35 */
  QFPROM_OEM_SPARE_3_REGION,                   /* 36 */
  
  /* Add above this */
  QFPROM_LAST_REGION_DUMMY,
  QFPROM_MAX_REGION_ENUM                = 0x7FFF /* To ensure it's 16 bits wide */
} QFPROM_REGION_NAME;

typedef struct
{
  /* region name; added only for readability */  
  ALIGN(4) QFPROM_REGION_NAME  region;

  /* how many rows the region takes */
  uint32_t                     size;

  /*  fec type of region */
  ALIGN(4) QFPROM_FEC_SCHEME   fec_type;

  /* raw address  of the region */
  uint32_t                      raw_base_addr;      

  /* corrected address of the region */
  uint32_t                      corr_base_addr;

  /* can we read this region    */
  uint32_t                      read_perm_mask;      

  /* can we write to this region    */
  uint32_t                      write_perm_mask;

  /* LSB or MSB  of the qfprom permission region */
  ALIGN(4) QFPROM_ROW           perm_reg_type;

  /* Region read-able/non-read-able */
  ALIGN(4) bool                 read_allowed; 

  /* Region index */
  uint32_t                      qfprom_region_index; 

} QFPROM_REGION_INFO;


typedef struct
{
    /* Row address */
    uint32_t raw_row_address;

    /* MSB row data */
    uint32_t row_data_msb;
    
    /* LSB row data */
    uint32_t row_data_lsb;
} write_row_type;


extern const QFPROM_REGION_INFO qfprom_region_data[];


/*===========================================================================

                      PUBLIC FUNCTION DECLARATIONS

===========================================================================*/


/*===========================================================================

**  Function :

** ==========================================================================
*/
/*!
*   qfprom_is_region_readable
* 
* @brief
*   This function shall return if the read permission for the region passed is available or not.
* 
* @par Dependencies
*    None
* 
* @param
*   region - The QFPROM region to be checked.
* 
* @retval boolean 
*                         - TRUE - permission is available 
*                         - FALSE - permission is not available 
* 
* @par Side Effects
* 
*/
bool     qfprom_is_region_readable
(
    QFPROM_REGION_NAME  region
);

/*===========================================================================

**  Function :    qfprom_is_region_writeable

** ==========================================================================
*/
/*!
*   qfprom_is_region_writeable
* 
* @brief
*   This function shall return if the write permission for the region passed is available or not.
* 
* @par Dependencies
*    None
* 
* @param
*   region - The QFPROM region to be checked.
* 
* @retval boolean 
*                         - TRUE - permission is available 
*                         - FALSE - permission is not available 
* 
* @par Side Effects
* 
*/
bool     qfprom_is_region_writeable
(
    QFPROM_REGION_NAME  region
);

/*===========================================================================

**  Function :

** ==========================================================================
*/
/*!
*   qfprom_is_region_fec_blown
*
* @brief
*   This function shall return if the FEC is blown for the region passed.
*
* @par Dependencies
*    None
*
* @param
*   region - The QFPROM region to be checked.
*
*   fec_status
*                         - TRUE - FEC is blown
*                         - FALSE - FEC is not blown
*
* @retval uint32_t  - QFPROM_NO_ERR - on success
*                     error_type    - if the operation is failure.
* 
* 
* @par Side Effects
*
*/
uint32_t     qfprom_is_region_fec_blown
(
    QFPROM_REGION_NAME  region,
    bool *fec_status
);

/*===========================================================================

**  Function : qfprom_get_region_name

** ==========================================================================
*/
/*!
* 
* @brief
*   This function gets the region of the given row address which need to be a RAW
*   address.
*  
* @par Dependencies
*   None.
*     
* @param:
*   uint32  address - Address of the Row.
*   QFPROM_ADDR_SPACE addr_type - Raw or Corrected address,
*   QFPROM_REGION_NAME* region_type - QFPROM Region of the Row address 
*                                                              passed.
* 
* @retval
* @retval: return - QFPROM_NO_ERR - if the operation is success.
*                           error_type           - if the operation is failure.                      
* @par Side Effects
*   None.
* 
*   
*/
uint32_t qfprom_get_region_name
 (
    uint32_t address,
    QFPROM_ADDR_SPACE addr_type,
    QFPROM_REGION_NAME* region_type    
 );


/*===========================================================================

FUNCTION  qfprom_check_convert_to_active_address

DESCRIPTION
    This function shall check the address the appropriate voltage dependent settings for the qfprom block.

PARAMETERS
    uint32   row_address  - Pointer to the row address to be checked 
    uint32   *active_row_address  - Pointer to the active row address after conversion 

DEPENDENCIES
  None.
 
RETURN VALUE
  uint32  Any Errors while setting up the system before blowing the fuses.

SIDE EFFECTS
  Disables the vreg being used.

===========================================================================*/

uint32_t
qfprom_check_convert_to_active_address
(
    uint32_t  row_address,
    uint32_t *active_row_address
);


/*===========================================================================

FUNCTION  qfprom_check_boundary_condition_for_row_address

DESCRIPTION
    This function shall check the boundary condition for the row address to be operated.

PARAMETERS
    uint32   row_address  - Row address to be checked 

DEPENDENCIES
  None.
 
RETURN VALUE
  uint32  Any Errors while setting up the system before blowing the fuses.

SIDE EFFECTS
  Disables the vreg being used.

===========================================================================*/

uint32_t
qfprom_check_boundary_condition_for_row_address
(
    uint32_t row_address
);


/*===========================================================================

**  Function : qfprom_get_region_index

** ==========================================================================
*/
/*!
* 
* @brief
*  This function gets the index of the region to which the given row address
*  belongs
*  
* @par Dependencies
*  None.
*     
* @param:
*  uint32 address - Address of the Row.
*  QFPROM_ADDR_SPACE addr_type - Raw or Corrected address,
*  uint32 *region_index - pointer to address location which will
*                          be filled in with index value by api.
* 
* @retval
* @retval: return - QFPROM_NO_ERR - if the operation is success.
*                   error_type    - if the operation is failure.                      
* @par Side Effects
*  None.
* 
*   
*/
uint32_t qfprom_get_region_index
(
    uint32_t address,
    QFPROM_ADDR_SPACE addr_type,
    uint32_t *region_index
);


/*===========================================================================
**  Function :    : qfprom_write

** ==========================================================================
*/
/*!
*
* @brief :  This function writes the actual raw data to the row
*
*
* @param  :
*                 uint32     raw_row_address,
*                 uint64    *raw_data_to_write     - Pointer to data.
*                 uintptr_t *pAddrErr             - first address with error
* @par Dependencies:
*
* @retval: return - QFPROM_NO_ERR - if the operation is success.
*                    error_type           - if the operation is failure.
*
* @par Side Effects:
* algo: Write the lower 32 bits and wait for the fuse write status to be ok.
*       Then write the upper 32 bits
*       If the write operation goes ok return QFPROM_NO_ERR else the error value.
*/
uint32_t qfprom_write
(
    uint32_t    raw_row_address,
    uint64_t  * raw_data_to_write,
    uintptr_t * pAddrErr
);


/*===========================================================================
**  Function :    : qfprom_write_check_row_pre_conditions

** ==========================================================================
*/
/*!
*
* @brief :  This function shall check if the row address is part of supported QFPROM Region
*            returns the status.
*
* @param  :
*                 uint32   raw_row_address,
*                 uint32   *row_data,           - Pointer to data.
*                 QFPROM_REGION_NAME* region_type
* @par Dependencies:
*
* @retval: return - QFPROM_NO_ERR - if the operation is success.
*                   error_type           - if the operation is failure.
*
*/
uint32_t qfprom_write_check_row_pre_conditions
(
    uint32_t   raw_row_address,
    QFPROM_REGION_NAME* region_type
);


/*===========================================================================

**  Function : qfprom_find_index

** ==========================================================================
*/
/*!
*
* @brief
*   This function finds the index of the fuse in the chain.
*
* @par Dependencies
*   None.
*
* @param:
*   QFPROM_REGION_NAME  This tells the region ID for which we want to find index.
*
* @retval
*   uint32  Index in qfprom table that matches qfpromId;
*           return QFPROM_INVALID_ENTRY in no match found.
*
* @par Side Effects
*   None.
*
*
*/
uint32_t qfprom_find_index(QFPROM_REGION_NAME qfpromRegion);

/*===========================================================================

**  Function : qfprom_get_corrected_address

** ==========================================================================
*/
/*!
* 
* @brief
*  This function gets the corrected address of the region to which the given raw 
*  row address belongs
*  
* @par Dependencies
*  None.
*     
* @param:
*  uint32 active_raw_address - Raw address of the Row.
*  uint32 *corr_address - pointer to address location which will
*                         be filled in with corr addr value by api.
* 
* @retval
* @retval: return - QFPROM_NO_ERR - if the operation is success.
*                   error_type    - if the operation is failure.
* @par Side Effects
*  None.
* 
*   
*/
uint32_t qfprom_get_corrected_address(uint32_t active_raw_address, uint32_t * corr_address);
#endif /* QFPROM_TARGET_NORD_H */
