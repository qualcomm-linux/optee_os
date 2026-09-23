/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * ICB configuration hardware register address definitions for
 * Lemans (QCS9075).
 */

#ifndef QTI_ICBCFG_HWIO_H
#define QTI_ICBCFG_HWIO_H

#include <stdint.h>

#include <io.h>
#include <types_ext.h>

/*
 * TFA-compatible replacements for TZ HWIO macros used by icbcfg_query.c
 * and icbcfg.c.
 *
 * All registers referenced below fall within a single absolute physical
 * span [ICBCFG_HWIO_BASE, ICBCFG_HWIO_BASE + ICBCFG_HWIO_SIZE). That span
 * is mapped lazily, on first use, by icbcfg_hwio_get_base() (defined in
 * icbcfg_query.c), which returns the (vaddr - paddr) delta so HWIO_IN()/
 * HWIO_OUT() can add it directly to an absolute physical register address
 * to get the mapped virtual address.
 *
 * HWIO_IN(addr) / HWIO_OUT(addr, val):
 *   Base register accessors. <addr> is an absolute physical address.
 *
 * HWIO_INXF(base, reg, field):
 *   Read the named field from register <reg> at absolute address <base>.
 *   <base> is the absolute address of the register (trans_bases[] already
 *   points directly at the register, so the offset is 0).
 *
 * HWIO_INXI(base, reg, index):
 *   Read indexed register <reg>[index] at absolute address <base>.
 *   The stride between consecutive entries is reg##_STRIDE bytes.
 */
#define ICBCFG_HWIO_BASE	0x01680000U
#define ICBCFG_HWIO_SIZE	0x23180000U

vaddr_t icbcfg_hwio_get_base(void);

#define HWIO_IN(addr) \
	io_read32(icbcfg_hwio_get_base() + (uintptr_t)(addr))
#define HWIO_OUT(addr, val) \
	io_write32(icbcfg_hwio_get_base() + (uintptr_t)(addr), (val))

#define HWIO_INXF(base, reg, field) \
	((HWIO_IN((uintptr_t)(base) + reg##_OFFSET) \
	  & reg##_##field##_BMSK) >> reg##_##field##_SHFT)

#define HWIO_INXI(base, reg, index) \
	HWIO_IN((uintptr_t)(base) + reg##_OFFSET + \
		(uint32_t)(index) * reg##_STRIDE)

/*
 * LLCC BEAC address-translator register layout.
 *
 * trans_bases[0] is set to the absolute address of the
 * LLCC_BEAC_ADDR_TRANSLATOR_CFG register for the selected LLCC slice
 * (e.g. LLCC0_LLCC_BEAC_ADDR_TRANSLATOR_CFG).  All offsets below are
 * relative to that same base.
 *
 * Register map (relative to LLCC_BEAC_ADDR_TRANSLATOR_CFG):
 *   +0x0000  ADDR_TRANSLATOR_CFG   (ERROR field at bit 0)
 *   +0x0010  ADDR_REGIONn_CFG0[n]  stride 0x20  (offset_lo[31:0])
 *   +0x0014  ADDR_REGIONn_CFG1[n]  stride 0x20  (offset_hi[31:0])
 *   +0x0018  ADDR_REGIONn_CFG2[n]  stride 0x20  (base_lo[31:0])
 *   +0x001C  ADDR_REGIONn_CFG3[n]  stride 0x20  (base_hi[31:0])
 */
#define LLCC_BEAC_ADDR_TRANSLATOR_CFG_OFFSET	0x0000U
#define LLCC_BEAC_ADDR_TRANSLATOR_CFG_ERROR_BMSK	0xFFFFFFFFU
#define LLCC_BEAC_ADDR_TRANSLATOR_CFG_ERROR_SHFT	0U

#define LLCC_BEAC_ADDR_REGIONn_CFG0_OFFSET	0x0010U
#define LLCC_BEAC_ADDR_REGIONn_CFG0_STRIDE	0x0020U

#define LLCC_BEAC_ADDR_REGIONn_CFG1_OFFSET	0x0014U
#define LLCC_BEAC_ADDR_REGIONn_CFG1_STRIDE	0x0020U

#define LLCC_BEAC_ADDR_REGIONn_CFG2_OFFSET	0x0018U
#define LLCC_BEAC_ADDR_REGIONn_CFG2_STRIDE	0x0020U

#define LLCC_BEAC_ADDR_REGIONn_CFG3_OFFSET	0x001CU
#define LLCC_BEAC_ADDR_REGIONn_CFG3_STRIDE	0x0020U

/*
 * Additional region registers for segments >= ADDL_REGION_START (6+).
 * Used by ICBCFG_ADDR_TRANS_LLCC when num_segments > ADDL_REGION_START.
 * addl_trans_bases[] points to the base of the additional translator block.
 */
#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG0_OFFSET	0x0010U
#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG0_STRIDE	0x0020U

#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG1_OFFSET	0x0014U
#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG1_STRIDE	0x0020U

#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG2_OFFSET	0x0018U
#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG2_STRIDE	0x0020U

#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG3_OFFSET	0x001CU
#define LLCC_BEAC_ADDR_ADDL_REGIONn_CFG3_STRIDE	0x0020U

/* MA (multi-address) region variants — same layout, different base offset */
#define LLCC_BEAC_MA_ADDR_REGIONn_CFG0_OFFSET	0x0110U
#define LLCC_BEAC_MA_ADDR_REGIONn_CFG0_STRIDE	0x0020U

#define LLCC_BEAC_MA_ADDR_REGIONn_CFG1_OFFSET	0x0114U
#define LLCC_BEAC_MA_ADDR_REGIONn_CFG1_STRIDE	0x0020U

#define LLCC_BEAC_MA_ADDR_REGIONn_CFG2_OFFSET	0x0118U
#define LLCC_BEAC_MA_ADDR_REGIONn_CFG2_STRIDE	0x0020U

#define LLCC_BEAC_MA_ADDR_REGIONn_CFG3_OFFSET	0x011CU
#define LLCC_BEAC_MA_ADDR_REGIONn_CFG3_STRIDE	0x0020U

/*
 * MemNOC address translator register layout.
 * Used for ICBCFG_ADDR_TRANS_NOC targets.
 *
 * Register map (relative to MEMNOC_ADDR_TRANSLATOR_CFG base):
 *   +0x0000  ADDR_TRANSLATOR_CFG   (ERROR field at bit 0)
 *   +0x0010  ADDR_TRANSLATOR_BASEn_LOW[n]   stride 0x10
 *   +0x0014  ADDR_TRANSLATOR_BASEn_HIGH[n]  stride 0x10
 *   +0x0018  ADDR_TRANSLATOR_OFFSETn_LOW[n] stride 0x10
 *   +0x001C  ADDR_TRANSLATOR_OFFSETn_HIGH[n] stride 0x10
 */
#define MEMNOC_ADDR_TRANSLATOR_CFG_OFFSET	0x0000U
#define MEMNOC_ADDR_TRANSLATOR_CFG_ERROR_BMSK	0xFFFFFFFFU
#define MEMNOC_ADDR_TRANSLATOR_CFG_ERROR_SHFT	0U

#define MEMNOC_ADDR_TRANSLATOR_BASEn_LOW_OFFSET	0x0010U
#define MEMNOC_ADDR_TRANSLATOR_BASEn_LOW_STRIDE	0x0010U

#define MEMNOC_ADDR_TRANSLATOR_BASEn_HIGH_OFFSET	0x0014U
#define MEMNOC_ADDR_TRANSLATOR_BASEn_HIGH_STRIDE	0x0010U

#define MEMNOC_ADDR_TRANSLATOR_OFFSETn_LOW_OFFSET	0x0018U
#define MEMNOC_ADDR_TRANSLATOR_OFFSETn_LOW_STRIDE	0x0010U

#define MEMNOC_ADDR_TRANSLATOR_OFFSETn_HIGH_OFFSET	0x001CU
#define MEMNOC_ADDR_TRANSLATOR_OFFSETn_HIGH_STRIDE	0x0010U

/*
 * MC ISU address translator register layout.
 * Used for ICBCFG_ADDR_TRANS_MC targets.
 *
 * Two register sets:
 *   REGIONn (segments 0..5):
 *     +0x0010  MC_ISU_ADDR_REGIONn_CFG0[n]  stride 0x20  (offset_lo)
 *     +0x0014  MC_ISU_ADDR_REGIONn_CFG1[n]  stride 0x20  (offset_hi)
 *     +0x0018  MC_ISU_ADDR_REGIONn_CFG2[n]  stride 0x20  (base_lo)
 *     +0x001C  MC_ISU_ADDR_REGIONn_CFG3[n]  stride 0x20  (base_hi)
 *   REGION_n (segments 6+, offset-only):
 *     +0x0110  MC_ISU_ADDR_REGION_n_CFG0[n] stride 0x10  (offset_lo)
 *     +0x0114  MC_ISU_ADDR_REGION_n_CFG1[n] stride 0x10  (offset_hi)
 */
#define MC_ISU_ADDR_TRANSLATOR_CFG_OFFSET	0x0000U
#define MC_ISU_ADDR_TRANSLATOR_CFG_ERROR_BMSK	0xFFFFFFFFU
#define MC_ISU_ADDR_TRANSLATOR_CFG_ERROR_SHFT	0U

#define MC_ISU_ADDR_REGIONn_CFG0_OFFSET	0x0010U
#define MC_ISU_ADDR_REGIONn_CFG0_STRIDE	0x0020U

#define MC_ISU_ADDR_REGIONn_CFG1_OFFSET	0x0014U
#define MC_ISU_ADDR_REGIONn_CFG1_STRIDE	0x0020U

#define MC_ISU_ADDR_REGIONn_CFG2_OFFSET	0x0018U
#define MC_ISU_ADDR_REGIONn_CFG2_STRIDE	0x0020U

#define MC_ISU_ADDR_REGIONn_CFG3_OFFSET	0x001CU
#define MC_ISU_ADDR_REGIONn_CFG3_STRIDE	0x0020U

#define MC_ISU_ADDR_REGION_n_CFG0_OFFSET	0x0110U
#define MC_ISU_ADDR_REGION_n_CFG0_STRIDE	0x0010U

#define MC_ISU_ADDR_REGION_n_CFG1_OFFSET	0x0114U
#define MC_ISU_ADDR_REGION_n_CFG1_STRIDE	0x0010U

/* Base addresses */
#define DDR_SS_BASE			0x24000000U
#define CORE_TOP_CSR_BASE		0x01f00000U

/*
 * Maximum physical DDR address for Lemans (36-bit address space).
 * Used as the upper bound of the mappable DDR region.
 */
#define SCL_DDR_MAX_ALLOWABLE_ADDR	0xfffffffffULL

/*
 * LLCC BEAC address-translator configuration registers.
 * LLCC[n]_LLCC_BEAC0_REG_BASE = DDR_SS_BASE + 0x00260000 + n * 0x100000
 * LLCC_BEAC_ADDR_TRANSLATOR_CFG offset within BEAC block = 0x8000
 */
#define LLCC0_LLCC_BEAC_ADDR_TRANSLATOR_CFG	(DDR_SS_BASE + 0x00268000U)
#define LLCC1_LLCC_BEAC_ADDR_TRANSLATOR_CFG	(DDR_SS_BASE + 0x00368000U)
#define LLCC2_LLCC_BEAC_ADDR_TRANSLATOR_CFG	(DDR_SS_BASE + 0x00468000U)
#define LLCC3_LLCC_BEAC_ADDR_TRANSLATOR_CFG	(DDR_SS_BASE + 0x00568000U)
#define LLCC4_LLCC_BEAC_ADDR_TRANSLATOR_CFG	(DDR_SS_BASE + 0x00668000U)
#define LLCC5_LLCC_BEAC_ADDR_TRANSLATOR_CFG	(DDR_SS_BASE + 0x00768000U)

/*
 * TCSR: DDR channel-select mode fuse/register.
 * TCSR_TCSR_REGS_REG_BASE = CORE_TOP_CSR_BASE + 0x000c0000
 */
#define TCSR_REGS_BASE			(CORE_TOP_CSR_BASE + 0x000c0000U)
#define TCSR_TIMEOUT_INTR_CH2_CH4_OR_CH6_STATUS \
	(TCSR_REGS_BASE + 0x8088U)
#define TCSR_DDR_CH_SEL_MODE_BMSK	0x3U

#endif /* QTI_ICBCFG_HWIO_H */
