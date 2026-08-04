/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <qfprom.h>
#include "msmhwiobase.h"
#include QFPROM_HWIOREG_INCLUDE_H
#include QFPROM_TARGET_INCLUDE_H

/*=============================================================================

            LOCAL DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains local definitions for constants, macros, types,
variables and other items needed by this module.

=============================================================================*/
/*
**  Array containing QFPROM data items that can be read and associated
**  registers, mask and shift values for the same.
*/
const QFPROM_REGION_INFO qfprom_region_data[] = {
  {
    QFPROM_MRC_REGION,                                                                      /* Region Name */
    1,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_MRC_LSB_ADDR,                                                          /* Raw address of the region */
    HWIO_QFPROM_CORR_MRC_LSB_ADDR,                                                         /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_MRC_READ_DISABLE_BMSK,                            /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_MRC_WRITE_DISABLE_BMSK,                          /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    3                                                                                       /* QFPROM Region Index */
  },
  {
    QFPROM_OEM_SECURITY_POLICY_REGION,                                                      /* Region Name */
    1,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_OEM_SECURITY_POLICY_LSB_ADDR,                                           /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_OEM_SECURITY_POLICY_LSB_ADDR,                                 /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_OEM_SECURITY_POLICY_READ_DISABLE_BMSK,            /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_OEM_SECURITY_POLICY_WRITE_DISABLE_BMSK,          /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    5                                                                                       /* QFPROM Region Index */
  },
  {
    QFPROM_WRITE_PERMISSIONS_REGION,                                                        /* Region Name */
    1,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_WRITE_PERMISSIONS_LSB_ADDR,                                             /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_WRITE_PERMISSIONS_LSB_ADDR,                                   /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_WRITE_PERMISSIONS_READ_DISABLE_BMSK,              /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_WRITE_PERMISSIONS_WRITE_DISABLE_BMSK,            /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    6                                                                                       /* QFPROM Region Index */
  },
  {
    QFPROM_READ_PERMISSIONS_REGION,                                                         /* Region Name */
    1,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_READ_PERMISSIONS_LSB_ADDR,                                              /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_READ_PERMISSIONS_LSB_ADDR,                                    /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_READ_PERMISSIONS_READ_DISABLE_BMSK,               /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_READ_PERMISSIONS_WRITE_DISABLE_BMSK,             /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    7                                                                                       /* QFPROM Region Index */
  },
  {
    QFPROM_FUSE_REDUNDANCY_ENABLE_REGION,                                                   /* Region Name */
    1,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_FUSE_REDUNDANCY_ENABLE_LSB_ADDR,                                        /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_FUSE_REDUNDANCY_ENABLE_LSB_ADDR,                              /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_FUSE_REDUNDANCY_ENABLE_READ_DISABLE_BMSK,         /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_FUSE_REDUNDANCY_ENABLE_WRITE_DISABLE_BMSK,       /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    8                                                                                       /* QFPROM Region Index */
  },
  {
    QFPROM_DEBUG_DISABLE_REGION,                                                            /* Region Name */
    2,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_DEBUG_DISABLE_ROW0_LSB_ADDR,                                                 /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_DEBUG_DISABLE_ROW0_LSB_ADDR,                                       /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_DEBUG_DISABLE_READ_DISABLE_BMSK,                  /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_DEBUG_DISABLE_WRITE_DISABLE_BMSK,                /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    9                                                                                       /* QFPROM Region Index */
  },
  {
    QFPROM_QC_CONFIG_REGION,                                                                /* Region Name */
    7,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_QC_CONFIG_ROW0_LSB_ADDR,                                                /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_QC_CONFIG_ROW0_LSB_ADDR,                                      /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_QC_CONFIG_READ_DISABLE_BMSK,                      /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_QC_CONFIG_WRITE_DISABLE_BMSK,                    /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    13                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_OEM_CONFIG_REGION,                                                               /* Region Name */
    6,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_OEM_CONFIG_ROW0_LSB_ADDR,                                               /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_OEM_CONFIG_ROW0_LSB_ADDR,                                     /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_OEM_CONFIG_READ_DISABLE_BMSK,                     /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_OEM_CONFIG_WRITE_DISABLE_BMSK,                   /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    14                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_FEATURE_CONFIG_REGION,                                                           /* Region Name */
    20,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_FEATURE_CONFIG_ROW0_LSB_ADDR,                                           /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_FEATURE_CONFIG_ROW0_LSB_ADDR,                                 /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_FEATURE_CONFIG_READ_DISABLE_BMSK,                 /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_FEATURE_CONFIG_WRITE_DISABLE_BMSK,               /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    15                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_ANTI_ROLLBACK_REGION,                                                            /* Region Name */
    50,                                                                                     /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_ANTIROLLBACK_ROW0_LSB_ADDR,                                             /* Raw address of the region */
    HWIO_QFPROM_CORR_ANTIROLLBACK_ROW0_LSB_ADDR,                                            /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_ANTIROLLBACK_READ_DISABLE_BMSK,                  /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_ANTIROLLBACK_WRITE_DISABLE_BMSK,                /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    17                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_QC_ECC_REGION,                                                                   /* Region Name */
    30,                                                                                     /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_QC_ECC_ROW0_LSB_ADDR,                                                   /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_QC_ECC_ROW0_LSB_ADDR,                                         /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_QC_ECC_READ_DISABLE_BMSK,                         /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_QC_ECC_WRITE_DISABLE_BMSK,                       /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    18                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_QC_SPARE_0_REGION,                                                               /* Region Name */
    15,                                                                                     /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_QC_SPARE_0_ROW0_LSB_ADDR,                                               /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_QC_SPARE_0_ROW0_LSB_ADDR,                                     /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_QC_SPARE_0_READ_DISABLE_BMSK,                      /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_QC_SPARE_0_WRITE_DISABLE_BMSK,                   /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    22                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_QC_SPARE_1_REGION,                                                               /* Region Name */
    15,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_QC_SPARE_1_ROW0_LSB_ADDR,                                               /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_QC_SPARE_1_ROW0_LSB_ADDR,                                     /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_QC_SPARE_1_READ_DISABLE_BMSK,                      /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_QC_SPARE_1_WRITE_DISABLE_BMSK,                   /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    23                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_QC_SPARE_2_REGION,                                                               /* Region Name */
    15,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_QC_SPARE_2_ROW0_LSB_ADDR,                                               /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_QC_SPARE_2_ROW0_LSB_ADDR,                                     /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_QC_SPARE_2_READ_DISABLE_BMSK,                      /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_QC_SPARE_2_WRITE_DISABLE_BMSK,                   /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    24                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_QC_SPARE_3_REGION,                                                               /* Region Name */
    17,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_QC_SPARE_3_ROW0_LSB_ADDR,                                               /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_QC_SPARE_3_ROW0_LSB_ADDR,                                     /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_QC_SPARE_3_READ_DISABLE_BMSK,                      /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_QC_SPARE_3_WRITE_DISABLE_BMSK,                   /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    25                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_MRC_HASH_REGION,                                                                 /* Region Name */
    10,                                                                                     /* Size - how many rows region takes */
    QFPROM_FEC_62_56,                                                                       /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_MRC_HASH_ROW0_LSB_ADDR,                                                 /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_MRC_HASH_ROW0_LSB_ADDR,                                       /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_MRC_HASH_READ_DISABLE_BMSK,                       /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_MRC_HASH_WRITE_DISABLE_BMSK,                     /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    28                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_OEM_PRODUCT_SEED_REGION,                                                         /* Region Name */
    3,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_62_56,                                                                       /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_OEM_PRODUCT_SEED_ROW0_LSB_ADDR,                                         /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_OEM_PRODUCT_SEED_ROW0_LSB_ADDR,                               /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_LSB_OEM_PRODUCT_SEED_READ_DISABLE_BMSK,               /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_LSB_OEM_PRODUCT_SEED_WRITE_DISABLE_BMSK,             /* Write permission */
    QFPROM_ROW_LSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    29                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_MEMORY_ACC_REGION, 
    44,
    QFPROM_FEC_NONE,
    HWIO_QFPROM_RAW_MEMORY_ACC_ROW0_LSB_ADDR,                                               /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_MEMORY_ACC_ROW0_LSB_ADDR,                                     /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_MSB_MEMORY_ACC_READ_DISABLE_BMSK,                     /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_MSB_MEMORY_ACC_WRITE_DISABLE_BMSK,                   /* Write permission */
    QFPROM_ROW_MSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    37                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_OEM_SPARE_0_REGION,                                                              /* Region Name */
    3,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_OEM_SPARE_0_ROW0_LSB_ADDR,                                              /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_OEM_SPARE_0_ROW0_LSB_ADDR,                                    /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_MSB_OEM_SPARE_0_READ_DISABLE_BMSK,                    /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_MSB_OEM_SPARE_0_WRITE_DISABLE_BMSK,                  /* Write permission */
    QFPROM_ROW_MSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    38                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_OEM_SPARE_1_REGION,                                                              /* Region Name */
    2,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_OEM_SPARE_1_ROW0_LSB_ADDR,                                              /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_OEM_SPARE_1_ROW0_LSB_ADDR,                                    /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_MSB_OEM_SPARE_1_READ_DISABLE_BMSK,                    /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_MSB_OEM_SPARE_1_WRITE_DISABLE_BMSK,                  /* Write permission */
    QFPROM_ROW_MSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    39                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_OEM_SPARE_2_REGION,                                                              /* Region Name */
    2,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_OEM_SPARE_2_ROW0_LSB_ADDR,                                              /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_OEM_SPARE_2_ROW0_LSB_ADDR,                                    /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_MSB_OEM_SPARE_2_READ_DISABLE_BMSK,                    /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_MSB_OEM_SPARE_2_WRITE_DISABLE_BMSK,                  /* Write permission */
    QFPROM_ROW_MSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    40                                                                                      /* QFPROM Region Index */
  },
  {
    QFPROM_OEM_SPARE_3_REGION,                                                              /* Region Name */
    2,                                                                                      /* Size - how many rows region takes */
    QFPROM_FEC_NONE,                                                                        /* FEC type - bits that contain FEC */
    HWIO_QFPROM_RAW_OEM_SPARE_3_ROW0_LSB_ADDR,                                              /* Raw address of the region */
    HWIO_REMAPPED_QFPROM_CORR_OEM_SPARE_3_ROW0_LSB_ADDR,                                    /* Corrected address of the region */
    HWIO_QFPROM_CORR_READ_PERMISSIONS_MSB_OEM_SPARE_3_READ_DISABLE_BMSK,                    /* Read permission */
    HWIO_QFPROM_CORR_WRITE_PERMISSIONS_MSB_OEM_SPARE_3_WRITE_DISABLE_BMSK,                  /* Write permission */
    QFPROM_ROW_MSB,                                                                         /* LSB or MSB of QFPROM permission region */
    true,                                                                                   /* Read Allow */
    41                                                                                      /* QFPROM Region Index */
  },
  
  /* Add above this entry */
  {
    QFPROM_LAST_REGION_DUMMY,
    0,
    QFPROM_FEC_NONE,
    0,
    0,
    0,
    0,
    QFPROM_ROW_LSB,
    false,
    QFPROM_LAST_REGION_DUMMY
  }
};
