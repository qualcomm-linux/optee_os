// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * QTI ICB (Interconnect Bus) driver - Lemans (QCS9075) target data.
 */

#include "icbuarbi.h"

/* HW NODE DECLARATIONS */
/* ACV */
static struct icb_hw_request bcm_acv_requests[1];
static struct icb_hw_node bcm_acv = {
	NULL,
	"ACV",
	ICB_HW_NODE_KIND_VECTOR,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_acv_requests},
	0
};

/* CE0 */
static struct icb_hw_request bcm_ce0_requests[2];
static struct icb_hw_node bcm_ce0 = {
	NULL,
	"CE0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 2, bcm_ce0_requests},
	0
};

/* CN0 */
static struct icb_hw_request bcm_cn0_requests[1];
static struct icb_hw_node bcm_cn0 = {
	NULL,
	"CN0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_cn0_requests},
	0
};

/* CN1 */
static struct icb_hw_request bcm_cn1_requests[3];
static struct icb_hw_node bcm_cn1 = {
	NULL,
	"CN1",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 3, bcm_cn1_requests},
	0
};

/* CN2 */
static struct icb_hw_request bcm_cn2_requests[4];
static struct icb_hw_node bcm_cn2 = {
	NULL,
	"CN2",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 4, bcm_cn2_requests},
	0
};

/* GNA0 */
static struct icb_hw_request bcm_gna0_requests[1];
static struct icb_hw_node bcm_gna0 = {
	NULL,
	"GNA0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_gna0_requests},
	0
};

/* GNB0 */
static struct icb_hw_request bcm_gnb0_requests[1];
static struct icb_hw_node bcm_gnb0 = {
	NULL,
	"GNB0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_gnb0_requests},
	0
};

/* HK0 */
static struct icb_hw_request bcm_hk0_requests[1];
static struct icb_hw_node bcm_hk0 = {
	NULL,
	"HK0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_hk0_requests},
	0
};

/* IP0 */
static struct icb_hw_request bcm_ip0_requests[1];
static struct icb_hw_node bcm_ip0 = {
	NULL,
	"IP0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_ip0_requests},
	0
};

/* MC0 */
static struct icb_hw_request bcm_mc0_requests[1];
static struct icb_hw_node bcm_mc0 = {
	NULL,
	"MC0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_mc0_requests},
	0
};

/* MM0 */
static struct icb_hw_request bcm_mm0_requests[2];
static struct icb_hw_node bcm_mm0 = {
	NULL,
	"MM0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 2, bcm_mm0_requests},
	0
};

/* NSA0 */
static struct icb_hw_request bcm_nsa0_requests[1];
static struct icb_hw_node bcm_nsa0 = {
	NULL,
	"NSA0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_nsa0_requests},
	0
};

/* NSA1 */
static struct icb_hw_request bcm_nsa1_requests[1];
static struct icb_hw_node bcm_nsa1 = {
	NULL,
	"NSA1",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_nsa1_requests},
	0
};

/* NSB0 */
static struct icb_hw_request bcm_nsb0_requests[1];
static struct icb_hw_node bcm_nsb0 = {
	NULL,
	"NSB0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_nsb0_requests},
	0
};

/* NSB1 */
static struct icb_hw_request bcm_nsb1_requests[1];
static struct icb_hw_node bcm_nsb1 = {
	NULL,
	"NSB1",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_nsb1_requests},
	0
};

/* PCI0 */
static struct icb_hw_request bcm_pci0_requests[1];
static struct icb_hw_node bcm_pci0 = {
	NULL,
	"PCI0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_pci0_requests},
	0
};

/* PKA0 */
static struct icb_hw_request bcm_pka0_requests[1];
static struct icb_hw_node bcm_pka0 = {
	NULL,
	"PKA0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_pka0_requests},
	0
};

/* QUP0 */
static struct icb_hw_request bcm_qup0_requests[1];
static struct icb_hw_node bcm_qup0 = {
	NULL,
	"QUP0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_qup0_requests},
	0
};

/* QUP1 */
static struct icb_hw_request bcm_qup1_requests[1];
static struct icb_hw_node bcm_qup1 = {
	NULL,
	"QUP1",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_qup1_requests},
	0
};

/* QUP2 */
static struct icb_hw_request bcm_qup2_requests[2];
static struct icb_hw_node bcm_qup2 = {
	NULL,
	"QUP2",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 2, bcm_qup2_requests},
	0
};

/* SH0 */
static struct icb_hw_request bcm_sh0_requests[1];
static struct icb_hw_node bcm_sh0 = {
	NULL,
	"SH0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_sh0_requests},
	0
};

/* SH2 */
static struct icb_hw_request bcm_sh2_requests[1];
static struct icb_hw_node bcm_sh2 = {
	NULL,
	"SH2",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_sh2_requests},
	0
};

/* SN0 */
static struct icb_hw_request bcm_sn0_requests[1];
static struct icb_hw_node bcm_sn0 = {
	NULL,
	"SN0",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 1, bcm_sn0_requests},
	0
};

/* SN3 */
static struct icb_hw_request bcm_sn3_requests[2];
static struct icb_hw_node bcm_sn3 = {
	NULL,
	"SN3",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 2, bcm_sn3_requests},
	0
};

/* SN4 */
static struct icb_hw_request bcm_sn4_requests[2];
static struct icb_hw_node bcm_sn4 = {
	NULL,
	"SN4",
	ICB_HW_NODE_KIND_BANDWIDTH,
	0,
	0,
	0,
	0,
	false,
	0,
	{0, 0},
	{0, 2, bcm_sn4_requests},
	0
};

/* MASTER DECLARATIONS */
/* CHM_APPS */
static struct icb_hw_node *master_chm_apps_hw_nodes[] = {
	&bcm_sh2,
};

static struct icb_master master_chm_apps = {
	ICBID_MASTER_APPSS_PROC,
	32,
	4,
	{0, 0},
	{0, 0, NULL},
	1,
	master_chm_apps_hw_nodes,
};

/* HKWM_CORE_MASTER */
static struct icb_master master_hkwm_core_master = {
	ICBID_MASTER_HWKM_CORE,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* IPA_CORE_MASTER */
static struct icb_master master_ipa_core_master = {
	ICBID_MASTER_IPA_CORE,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* LLCC_MC */
static struct icb_master master_llcc_mc = {
	ICBID_MASTER_LLCC,
	4,
	8,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* PKE_CORE_MASTER */
static struct icb_master master_pke_core_master = {
	ICBID_MASTER_PKA_CORE,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QHM_QUP0 */
static struct icb_master master_qhm_qup0 = {
	ICBID_MASTER_QUP_0,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QHM_QUP1 */
static struct icb_master master_qhm_qup1 = {
	ICBID_MASTER_QUP_1,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QHM_QUP2 */
static struct icb_master master_qhm_qup2 = {
	ICBID_MASTER_QUP_2,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNM_AGGRE1_NOC */
static struct icb_hw_node *master_qnm_aggre1_noc_hw_nodes[] = {
	&bcm_sn3,
};

static struct icb_master master_qnm_aggre1_noc = {
	ICBID_MASTER_A1NOC_SNOC,
	32,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qnm_aggre1_noc_hw_nodes,
};

/* QNM_AGGRE2_NOC */
static struct icb_hw_node *master_qnm_aggre2_noc_hw_nodes[] = {
	&bcm_sn4,
};

static struct icb_master master_qnm_aggre2_noc = {
	ICBID_MASTER_A2NOC_SNOC,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qnm_aggre2_noc_hw_nodes,
};

/* QNM_CMPNOC0 */
static struct icb_master master_qnm_cmpnoc0 = {
	ICBID_MASTER_COMPUTE_NOC,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNM_CMPNOC1 */
static struct icb_master master_qnm_cmpnoc1 = {
	ICBID_MASTER_COMPUTE_NOC_1,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNM_GEMNOC_CNOC */
static struct icb_hw_node *master_qnm_gemnoc_cnoc_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_master master_qnm_gemnoc_cnoc = {
	ICBID_MASTER_GEM_NOC_CNOC,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qnm_gemnoc_cnoc_hw_nodes,
};

/* QNM_GPDSP_SAIL */
static struct icb_master master_qnm_gpdsp_sail = {
	ICBID_MASTER_GPDSP_SAIL,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNM_MDP0_0 */
static struct icb_hw_node *master_qnm_mdp0_0_hw_nodes[] = {
	&bcm_mm0,
};

static struct icb_master master_qnm_mdp0_0 = {
	ICBID_MASTER_MDP0,
	32,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qnm_mdp0_0_hw_nodes,
};

/* QNM_MNOC_HF */
static struct icb_master master_qnm_mnoc_hf = {
	ICBID_MASTER_MNOC_HF_MEM_NOC,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNM_PCIE */
static struct icb_master master_qnm_pcie = {
	ICBID_MASTER_ANOC_PCIE_GEM_NOC,
	32,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNM_SNOC_SF */
static struct icb_master master_qnm_snoc_sf = {
	ICBID_MASTER_SNOC_SF_MEM_NOC,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QUP0_CORE_MASTER */
static struct icb_master master_qup0_core_master = {
	ICBID_MASTER_QUP_CORE_0,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QUP1_CORE_MASTER */
static struct icb_master master_qup1_core_master = {
	ICBID_MASTER_QUP_CORE_1,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QUP2_CORE_MASTER */
static struct icb_master master_qup2_core_master = {
	ICBID_MASTER_QUP_CORE_2,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QUP3_CORE_MASTER */
static struct icb_master master_qup3_core_master = {
	ICBID_MASTER_QUP_CORE_3,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QXM_CRYPTO_0 */
static struct icb_hw_node *master_qxm_crypto_0_hw_nodes[] = {
	&bcm_ce0,
};

static struct icb_master master_qxm_crypto_0 = {
	ICBID_MASTER_CRYPTO_CORE0,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qxm_crypto_0_hw_nodes,
};

/* QXM_CRYPTO_1 */
static struct icb_hw_node *master_qxm_crypto_1_hw_nodes[] = {
	&bcm_ce0,
};

static struct icb_master master_qxm_crypto_1 = {
	ICBID_MASTER_CRYPTO_CORE1,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qxm_crypto_1_hw_nodes,
};

/* QXM_DSP0 */
static struct icb_hw_node *master_qxm_dsp0_hw_nodes[] = {
	&bcm_gna0,
};

static struct icb_master master_qxm_dsp0 = {
	ICBID_MASTER_DSP0,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qxm_dsp0_hw_nodes,
};

/* QXM_DSP1 */
static struct icb_hw_node *master_qxm_dsp1_hw_nodes[] = {
	&bcm_gnb0,
};

static struct icb_master master_qxm_dsp1 = {
	ICBID_MASTER_DSP1,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qxm_dsp1_hw_nodes,
};

/* QXM_NSP */
static struct icb_hw_node *master_qxm_nsp_hw_nodes[] = {
	&bcm_nsa1,
};

static struct icb_master master_qxm_nsp = {
	ICBID_MASTER_CDSP_PROC,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qxm_nsp_hw_nodes,
};

/* QXM_NSPB */
static struct icb_hw_node *master_qxm_nspb_hw_nodes[] = {
	&bcm_nsb1,
};

static struct icb_master master_qxm_nspb = {
	ICBID_MASTER_CDSP_PROC_B,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	1,
	master_qxm_nspb_hw_nodes,
};

/* QXM_QUP3 */
static struct icb_master master_qxm_qup3 = {
	ICBID_MASTER_QUP_3,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* XM_EMAC_0 */
static struct icb_master master_xm_emac_0 = {
	ICBID_MASTER_EMAC,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* XM_EMAC_1 */
static struct icb_master master_xm_emac_1 = {
	ICBID_MASTER_EMAC_1,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* XM_PCIE3_0 */
static struct icb_master master_xm_pcie3_0 = {
	ICBID_MASTER_PCIE_0,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* XM_PCIE3_1 */
static struct icb_master master_xm_pcie3_1 = {
	ICBID_MASTER_PCIE_1,
	32,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* SLAVE DECLARATIONS */
/* EBI */
static struct icb_hw_node *slave_ebi_hw_nodes[] = {
	&bcm_mc0,
	&bcm_acv,
};

static struct icb_slave slave_ebi = {
	ICBID_SLAVE_EBI1,
	4,
	8,
	{0, 0},
	{0, 0, NULL},
	2,
	slave_ebi_hw_nodes,
};

/* HKWM_CORE_SLAVE */
static struct icb_hw_node *slave_hkwm_core_slave_hw_nodes[] = {
	&bcm_hk0,
};

static struct icb_slave slave_hkwm_core_slave = {
	ICBID_SLAVE_HWKM_CORE,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_hkwm_core_slave_hw_nodes,
};

/* IPA_CORE_SLAVE */
static struct icb_hw_node *slave_ipa_core_slave_hw_nodes[] = {
	&bcm_ip0,
};

static struct icb_slave slave_ipa_core_slave = {
	ICBID_SLAVE_IPA_CORE,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_ipa_core_slave_hw_nodes,
};

/* PKE_CORE_SLAVE */
static struct icb_hw_node *slave_pke_core_slave_hw_nodes[] = {
	&bcm_pka0,
};

static struct icb_slave slave_pke_core_slave = {
	ICBID_SLAVE_PKA_CORE,
	8,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_pke_core_slave_hw_nodes,
};

/* QHS_CRYPTO0_CFG */
static struct icb_hw_node *slave_qhs_crypto0_cfg_hw_nodes[] = {
	&bcm_cn1,
};

static struct icb_slave slave_qhs_crypto0_cfg = {
	ICBID_SLAVE_CRYPTO_0_CFG,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qhs_crypto0_cfg_hw_nodes,
};

/* QHS_HWKM */
static struct icb_hw_node *slave_qhs_hwkm_hw_nodes[] = {
	&bcm_cn1,
};

static struct icb_slave slave_qhs_hwkm = {
	ICBID_SLAVE_HWKM,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qhs_hwkm_hw_nodes,
};

/* QHS_PKE_WRAPPER_CFG */
static struct icb_hw_node *slave_qhs_pke_wrapper_cfg_hw_nodes[] = {
	&bcm_cn1,
};

static struct icb_slave slave_qhs_pke_wrapper_cfg = {
	ICBID_SLAVE_PKA_WRAPPER_CFG,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qhs_pke_wrapper_cfg_hw_nodes,
};

/* QHS_QUP0 */
static struct icb_hw_node *slave_qhs_qup0_hw_nodes[] = {
	&bcm_cn2,
};

static struct icb_slave slave_qhs_qup0 = {
	ICBID_SLAVE_QUP_0,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qhs_qup0_hw_nodes,
};

/* QHS_QUP1 */
static struct icb_hw_node *slave_qhs_qup1_hw_nodes[] = {
	&bcm_cn2,
};

static struct icb_slave slave_qhs_qup1 = {
	ICBID_SLAVE_QUP_1,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qhs_qup1_hw_nodes,
};

/* QHS_QUP2 */
static struct icb_hw_node *slave_qhs_qup2_hw_nodes[] = {
	&bcm_cn2,
};

static struct icb_slave slave_qhs_qup2 = {
	ICBID_SLAVE_QUP_2,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qhs_qup2_hw_nodes,
};

/* QHS_QUP3 */
static struct icb_hw_node *slave_qhs_qup3_hw_nodes[] = {
	&bcm_cn2,
};

static struct icb_slave slave_qhs_qup3 = {
	ICBID_SLAVE_QUP_3,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qhs_qup3_hw_nodes,
};

/* QNS_A1NOC_SNOC */
static struct icb_hw_node *slave_qns_a1noc_snoc_hw_nodes[] = {
	&bcm_sn3,
};

static struct icb_slave slave_qns_a1noc_snoc = {
	ICBID_SLAVE_A1NOC_SNOC,
	32,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_a1noc_snoc_hw_nodes,
};

/* QNS_A2NOC_SNOC */
static struct icb_hw_node *slave_qns_a2noc_snoc_hw_nodes[] = {
	&bcm_sn4,
};

static struct icb_slave slave_qns_a2noc_snoc = {
	ICBID_SLAVE_A2NOC_SNOC,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_a2noc_snoc_hw_nodes,
};

/* QNS_GEM_NOC_CNOC */
static struct icb_slave slave_qns_gem_noc_cnoc = {
	ICBID_SLAVE_GEM_NOC_CNOC,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNS_GEMNOC_SF */
static struct icb_hw_node *slave_qns_gemnoc_sf_hw_nodes[] = {
	&bcm_sn0,
};

static struct icb_slave slave_qns_gemnoc_sf = {
	ICBID_SLAVE_SNOC_GEM_NOC_SF,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_gemnoc_sf_hw_nodes,
};

/* QNS_GP_DSP_SAIL_NOC */
static struct icb_slave slave_qns_gp_dsp_sail_noc = {
	ICBID_SLAVE_GP_DSP_SAIL_NOC,
	16,
	1,
	{0, 0},
	{0, 0, NULL},
	0,
	NULL,
};

/* QNS_LLCC */
static struct icb_hw_node *slave_qns_llcc_hw_nodes[] = {
	&bcm_sh0,
};

static struct icb_slave slave_qns_llcc = {
	ICBID_SLAVE_LLCC,
	16,
	6,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_llcc_hw_nodes,
};

/* QNS_MEM_NOC_HF */
static struct icb_hw_node *slave_qns_mem_noc_hf_hw_nodes[] = {
	&bcm_mm0,
};

static struct icb_slave slave_qns_mem_noc_hf = {
	ICBID_SLAVE_MNOC_HF_MEM_NOC,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_mem_noc_hf_hw_nodes,
};

/* QNS_NSP_GEMNOC */
static struct icb_hw_node *slave_qns_nsp_gemnoc_hw_nodes[] = {
	&bcm_nsa0,
};

static struct icb_slave slave_qns_nsp_gemnoc = {
	ICBID_SLAVE_CDSP_MEM_NOC,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_nsp_gemnoc_hw_nodes,
};

/* QNS_NSPB_GEMNOC */
static struct icb_hw_node *slave_qns_nspb_gemnoc_hw_nodes[] = {
	&bcm_nsb0,
};

static struct icb_slave slave_qns_nspb_gemnoc = {
	ICBID_SLAVE_CDSPB_MEM_NOC,
	32,
	2,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_nspb_gemnoc_hw_nodes,
};

/* QNS_PCIE_MEM_NOC */
static struct icb_hw_node *slave_qns_pcie_mem_noc_hw_nodes[] = {
	&bcm_pci0,
};

static struct icb_slave slave_qns_pcie_mem_noc = {
	ICBID_SLAVE_ANOC_PCIE_GEM_NOC,
	32,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qns_pcie_mem_noc_hw_nodes,
};

/* QUP0_CORE_SLAVE */
static struct icb_hw_node *slave_qup0_core_slave_hw_nodes[] = {
	&bcm_qup0,
};

static struct icb_slave slave_qup0_core_slave = {
	ICBID_SLAVE_QUP_CORE_0,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qup0_core_slave_hw_nodes,
};

/* QUP1_CORE_SLAVE */
static struct icb_hw_node *slave_qup1_core_slave_hw_nodes[] = {
	&bcm_qup1,
};

static struct icb_slave slave_qup1_core_slave = {
	ICBID_SLAVE_QUP_CORE_1,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qup1_core_slave_hw_nodes,
};

/* QUP2_CORE_SLAVE */
static struct icb_hw_node *slave_qup2_core_slave_hw_nodes[] = {
	&bcm_qup2,
};

static struct icb_slave slave_qup2_core_slave = {
	ICBID_SLAVE_QUP_CORE_2,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qup2_core_slave_hw_nodes,
};

/* QUP3_CORE_SLAVE */
static struct icb_hw_node *slave_qup3_core_slave_hw_nodes[] = {
	&bcm_qup2,
};

static struct icb_slave slave_qup3_core_slave = {
	ICBID_SLAVE_QUP_CORE_3,
	4,
	1,
	{0, 0},
	{0, 0, NULL},
	1,
	slave_qup3_core_slave_hw_nodes,
};

/* TOPOLOGY ROUTE DECLARATIONS */
static struct icb_pair route_chm_apps_ebi_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_chm_apps_ebi = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_EBI1,
	2,
	route_chm_apps_ebi_hops
};

static struct icb_pair route_chm_apps_qhs_crypto0_cfg_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_crypto0_cfg
	},
};

static struct icb_route route_chm_apps_qhs_crypto0_cfg = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_CRYPTO_0_CFG,
	2,
	route_chm_apps_qhs_crypto0_cfg_hops
};

static struct icb_pair route_chm_apps_qhs_hwkm_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_hwkm
	},
};

static struct icb_route route_chm_apps_qhs_hwkm = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_HWKM,
	2,
	route_chm_apps_qhs_hwkm_hops
};

static struct icb_pair route_chm_apps_qhs_pke_wrapper_cfg_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_pke_wrapper_cfg
	},
};

static struct icb_route route_chm_apps_qhs_pke_wrapper_cfg = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_PKA_WRAPPER_CFG,
	2,
	route_chm_apps_qhs_pke_wrapper_cfg_hops
};

static struct icb_pair route_chm_apps_qhs_qup0_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup0
	},
};

static struct icb_route route_chm_apps_qhs_qup0 = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QUP_0,
	2,
	route_chm_apps_qhs_qup0_hops
};

static struct icb_pair route_chm_apps_qhs_qup1_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup1
	},
};

static struct icb_route route_chm_apps_qhs_qup1 = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QUP_1,
	2,
	route_chm_apps_qhs_qup1_hops
};

static struct icb_pair route_chm_apps_qhs_qup2_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup2
	},
};

static struct icb_route route_chm_apps_qhs_qup2 = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QUP_2,
	2,
	route_chm_apps_qhs_qup2_hops
};

static struct icb_pair route_chm_apps_qhs_qup3_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup3
	},
};

static struct icb_route route_chm_apps_qhs_qup3 = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QUP_3,
	2,
	route_chm_apps_qhs_qup3_hops
};

static struct icb_pair route_chm_apps_qns_llcc_hops[1] = {
	{
		&master_chm_apps,
		&slave_qns_llcc
	},
};

static struct icb_route route_chm_apps_qns_llcc = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_LLCC,
	1,
	route_chm_apps_qns_llcc_hops
};

static struct icb_pair route_hkwm_core_master_hkwm_core_slave_hops[1] = {
	{
		&master_hkwm_core_master,
		&slave_hkwm_core_slave
	},
};

static struct icb_route route_hkwm_core_master_hkwm_core_slave = {
	ICBID_MASTER_HWKM_CORE,
	ICBID_SLAVE_HWKM_CORE,
	1,
	route_hkwm_core_master_hkwm_core_slave_hops
};

static struct icb_pair route_ipa_core_master_ipa_core_slave_hops[1] = {
	{
		&master_ipa_core_master,
		&slave_ipa_core_slave
	},
};

static struct icb_route route_ipa_core_master_ipa_core_slave = {
	ICBID_MASTER_IPA_CORE,
	ICBID_SLAVE_IPA_CORE,
	1,
	route_ipa_core_master_ipa_core_slave_hops
};

static struct icb_pair route_llcc_mc_ebi_hops[1] = {
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_llcc_mc_ebi = {
	ICBID_MASTER_LLCC,
	ICBID_SLAVE_EBI1,
	1,
	route_llcc_mc_ebi_hops
};

static struct icb_pair route_pke_core_master_pke_core_slave_hops[1] = {
	{
		&master_pke_core_master,
		&slave_pke_core_slave
	},
};

static struct icb_route route_pke_core_master_pke_core_slave = {
	ICBID_MASTER_PKA_CORE,
	ICBID_SLAVE_PKA_CORE,
	1,
	route_pke_core_master_pke_core_slave_hops
};

static struct icb_pair route_qhm_qup0_ebi_hops[4] = {
	{
		&master_qhm_qup0,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qhm_qup0_ebi = {
	ICBID_MASTER_QUP_0,
	ICBID_SLAVE_EBI1,
	4,
	route_qhm_qup0_ebi_hops
};

static struct icb_pair route_qhm_qup0_qns_llcc_hops[3] = {
	{
		&master_qhm_qup0,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qhm_qup0_qns_llcc = {
	ICBID_MASTER_QUP_0,
	ICBID_SLAVE_LLCC,
	3,
	route_qhm_qup0_qns_llcc_hops
};

static struct icb_pair route_qhm_qup1_ebi_hops[4] = {
	{
		&master_qhm_qup1,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qhm_qup1_ebi = {
	ICBID_MASTER_QUP_1,
	ICBID_SLAVE_EBI1,
	4,
	route_qhm_qup1_ebi_hops
};

static struct icb_pair route_qhm_qup1_qns_llcc_hops[3] = {
	{
		&master_qhm_qup1,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qhm_qup1_qns_llcc = {
	ICBID_MASTER_QUP_1,
	ICBID_SLAVE_LLCC,
	3,
	route_qhm_qup1_qns_llcc_hops
};

static struct icb_pair route_qhm_qup2_ebi_hops[4] = {
	{
		&master_qhm_qup2,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qhm_qup2_ebi = {
	ICBID_MASTER_QUP_2,
	ICBID_SLAVE_EBI1,
	4,
	route_qhm_qup2_ebi_hops
};

static struct icb_pair route_qhm_qup2_qns_llcc_hops[3] = {
	{
		&master_qhm_qup2,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qhm_qup2_qns_llcc = {
	ICBID_MASTER_QUP_2,
	ICBID_SLAVE_LLCC,
	3,
	route_qhm_qup2_qns_llcc_hops
};

static struct icb_pair route_qnm_mdp0_0_ebi_hops[3] = {
	{
		&master_qnm_mdp0_0,
		&slave_qns_mem_noc_hf
	},
	{
		&master_qnm_mnoc_hf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qnm_mdp0_0_ebi = {
	ICBID_MASTER_MDP0,
	ICBID_SLAVE_EBI1,
	3,
	route_qnm_mdp0_0_ebi_hops
};

static struct icb_pair route_qnm_mdp0_0_qns_llcc_hops[2] = {
	{
		&master_qnm_mdp0_0,
		&slave_qns_mem_noc_hf
	},
	{
		&master_qnm_mnoc_hf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qnm_mdp0_0_qns_llcc = {
	ICBID_MASTER_MDP0,
	ICBID_SLAVE_LLCC,
	2,
	route_qnm_mdp0_0_qns_llcc_hops
};

static struct icb_pair route_qnm_mdp0_0_qns_mem_noc_hf_hops[1] = {
	{
		&master_qnm_mdp0_0,
		&slave_qns_mem_noc_hf
	},
};

static struct icb_route route_qnm_mdp0_0_qns_mem_noc_hf = {
	ICBID_MASTER_MDP0,
	ICBID_SLAVE_MNOC_HF_MEM_NOC,
	1,
	route_qnm_mdp0_0_qns_mem_noc_hf_hops
};

static struct icb_pair route_qup0_core_master_qup0_core_slave_hops[1] = {
	{
		&master_qup0_core_master,
		&slave_qup0_core_slave
	},
};

static struct icb_route route_qup0_core_master_qup0_core_slave = {
	ICBID_MASTER_QUP_CORE_0,
	ICBID_SLAVE_QUP_CORE_0,
	1,
	route_qup0_core_master_qup0_core_slave_hops
};

static struct icb_pair route_qup1_core_master_qup1_core_slave_hops[1] = {
	{
		&master_qup1_core_master,
		&slave_qup1_core_slave
	},
};

static struct icb_route route_qup1_core_master_qup1_core_slave = {
	ICBID_MASTER_QUP_CORE_1,
	ICBID_SLAVE_QUP_CORE_1,
	1,
	route_qup1_core_master_qup1_core_slave_hops
};

static struct icb_pair route_qup2_core_master_qup2_core_slave_hops[1] = {
	{
		&master_qup2_core_master,
		&slave_qup2_core_slave
	},
};

static struct icb_route route_qup2_core_master_qup2_core_slave = {
	ICBID_MASTER_QUP_CORE_2,
	ICBID_SLAVE_QUP_CORE_2,
	1,
	route_qup2_core_master_qup2_core_slave_hops
};

static struct icb_pair route_qup3_core_master_qup3_core_slave_hops[1] = {
	{
		&master_qup3_core_master,
		&slave_qup3_core_slave
	},
};

static struct icb_route route_qup3_core_master_qup3_core_slave = {
	ICBID_MASTER_QUP_CORE_3,
	ICBID_SLAVE_QUP_CORE_3,
	1,
	route_qup3_core_master_qup3_core_slave_hops
};

static struct icb_pair route_qxm_crypto_0_ebi_hops[4] = {
	{
		&master_qxm_crypto_0,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qxm_crypto_0_ebi = {
	ICBID_MASTER_CRYPTO_CORE0,
	ICBID_SLAVE_EBI1,
	4,
	route_qxm_crypto_0_ebi_hops
};

static struct icb_pair route_qxm_crypto_0_qns_llcc_hops[3] = {
	{
		&master_qxm_crypto_0,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qxm_crypto_0_qns_llcc = {
	ICBID_MASTER_CRYPTO_CORE0,
	ICBID_SLAVE_LLCC,
	3,
	route_qxm_crypto_0_qns_llcc_hops
};

static struct icb_pair route_qxm_crypto_1_ebi_hops[4] = {
	{
		&master_qxm_crypto_1,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qxm_crypto_1_ebi = {
	ICBID_MASTER_CRYPTO_CORE1,
	ICBID_SLAVE_EBI1,
	4,
	route_qxm_crypto_1_ebi_hops
};

static struct icb_pair route_qxm_crypto_1_qns_llcc_hops[3] = {
	{
		&master_qxm_crypto_1,
		&slave_qns_a2noc_snoc
	},
	{
		&master_qnm_aggre2_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qxm_crypto_1_qns_llcc = {
	ICBID_MASTER_CRYPTO_CORE1,
	ICBID_SLAVE_LLCC,
	3,
	route_qxm_crypto_1_qns_llcc_hops
};

static struct icb_pair route_qxm_dsp0_ebi_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qxm_dsp0_ebi = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_EBI1,
	3,
	route_qxm_dsp0_ebi_hops
};

static struct icb_pair route_qxm_dsp0_qhs_crypto0_cfg_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_crypto0_cfg
	},
};

static struct icb_route route_qxm_dsp0_qhs_crypto0_cfg = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_CRYPTO_0_CFG,
	3,
	route_qxm_dsp0_qhs_crypto0_cfg_hops
};

static struct icb_pair route_qxm_dsp0_qhs_hwkm_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_hwkm
	},
};

static struct icb_route route_qxm_dsp0_qhs_hwkm = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_HWKM,
	3,
	route_qxm_dsp0_qhs_hwkm_hops
};

static struct icb_pair route_qxm_dsp0_qhs_pke_wrapper_cfg_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_pke_wrapper_cfg
	},
};

static struct icb_route route_qxm_dsp0_qhs_pke_wrapper_cfg = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_PKA_WRAPPER_CFG,
	3,
	route_qxm_dsp0_qhs_pke_wrapper_cfg_hops
};

static struct icb_pair route_qxm_dsp0_qhs_qup0_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup0
	},
};

static struct icb_route route_qxm_dsp0_qhs_qup0 = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_QUP_0,
	3,
	route_qxm_dsp0_qhs_qup0_hops
};

static struct icb_pair route_qxm_dsp0_qhs_qup1_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup1
	},
};

static struct icb_route route_qxm_dsp0_qhs_qup1 = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_QUP_1,
	3,
	route_qxm_dsp0_qhs_qup1_hops
};

static struct icb_pair route_qxm_dsp0_qhs_qup2_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup2
	},
};

static struct icb_route route_qxm_dsp0_qhs_qup2 = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_QUP_2,
	3,
	route_qxm_dsp0_qhs_qup2_hops
};

static struct icb_pair route_qxm_dsp0_qhs_qup3_hops[3] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup3
	},
};

static struct icb_route route_qxm_dsp0_qhs_qup3 = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_QUP_3,
	3,
	route_qxm_dsp0_qhs_qup3_hops
};

static struct icb_pair route_qxm_dsp0_qns_llcc_hops[2] = {
	{
		&master_qxm_dsp0,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_llcc
	},
};

static struct icb_route route_qxm_dsp0_qns_llcc = {
	ICBID_MASTER_DSP0,
	ICBID_SLAVE_LLCC,
	2,
	route_qxm_dsp0_qns_llcc_hops
};

static struct icb_pair route_qxm_dsp1_ebi_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qxm_dsp1_ebi = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_EBI1,
	3,
	route_qxm_dsp1_ebi_hops
};

static struct icb_pair route_qxm_dsp1_qhs_crypto0_cfg_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_crypto0_cfg
	},
};

static struct icb_route route_qxm_dsp1_qhs_crypto0_cfg = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_CRYPTO_0_CFG,
	3,
	route_qxm_dsp1_qhs_crypto0_cfg_hops
};

static struct icb_pair route_qxm_dsp1_qhs_hwkm_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_hwkm
	},
};

static struct icb_route route_qxm_dsp1_qhs_hwkm = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_HWKM,
	3,
	route_qxm_dsp1_qhs_hwkm_hops
};

static struct icb_pair route_qxm_dsp1_qhs_pke_wrapper_cfg_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_pke_wrapper_cfg
	},
};

static struct icb_route route_qxm_dsp1_qhs_pke_wrapper_cfg = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_PKA_WRAPPER_CFG,
	3,
	route_qxm_dsp1_qhs_pke_wrapper_cfg_hops
};

static struct icb_pair route_qxm_dsp1_qhs_qup0_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup0
	},
};

static struct icb_route route_qxm_dsp1_qhs_qup0 = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_QUP_0,
	3,
	route_qxm_dsp1_qhs_qup0_hops
};

static struct icb_pair route_qxm_dsp1_qhs_qup1_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup1
	},
};

static struct icb_route route_qxm_dsp1_qhs_qup1 = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_QUP_1,
	3,
	route_qxm_dsp1_qhs_qup1_hops
};

static struct icb_pair route_qxm_dsp1_qhs_qup2_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup2
	},
};

static struct icb_route route_qxm_dsp1_qhs_qup2 = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_QUP_2,
	3,
	route_qxm_dsp1_qhs_qup2_hops
};

static struct icb_pair route_qxm_dsp1_qhs_qup3_hops[3] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qup3
	},
};

static struct icb_route route_qxm_dsp1_qhs_qup3 = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_QUP_3,
	3,
	route_qxm_dsp1_qhs_qup3_hops
};

static struct icb_pair route_qxm_dsp1_qns_llcc_hops[2] = {
	{
		&master_qxm_dsp1,
		&slave_qns_gp_dsp_sail_noc
	},
	{
		&master_qnm_gpdsp_sail,
		&slave_qns_llcc
	},
};

static struct icb_route route_qxm_dsp1_qns_llcc = {
	ICBID_MASTER_DSP1,
	ICBID_SLAVE_LLCC,
	2,
	route_qxm_dsp1_qns_llcc_hops
};

static struct icb_pair route_qxm_nspb_ebi_hops[3] = {
	{
		&master_qxm_nspb,
		&slave_qns_nspb_gemnoc
	},
	{
		&master_qnm_cmpnoc1,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qxm_nspb_ebi = {
	ICBID_MASTER_CDSP_PROC_B,
	ICBID_SLAVE_EBI1,
	3,
	route_qxm_nspb_ebi_hops
};

static struct icb_pair route_qxm_nspb_qns_llcc_hops[2] = {
	{
		&master_qxm_nspb,
		&slave_qns_nspb_gemnoc
	},
	{
		&master_qnm_cmpnoc1,
		&slave_qns_llcc
	},
};

static struct icb_route route_qxm_nspb_qns_llcc = {
	ICBID_MASTER_CDSP_PROC_B,
	ICBID_SLAVE_LLCC,
	2,
	route_qxm_nspb_qns_llcc_hops
};

static struct icb_pair route_qxm_nsp_ebi_hops[3] = {
	{
		&master_qxm_nsp,
		&slave_qns_nsp_gemnoc
	},
	{
		&master_qnm_cmpnoc0,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qxm_nsp_ebi = {
	ICBID_MASTER_CDSP_PROC,
	ICBID_SLAVE_EBI1,
	3,
	route_qxm_nsp_ebi_hops
};

static struct icb_pair route_qxm_nsp_qns_llcc_hops[2] = {
	{
		&master_qxm_nsp,
		&slave_qns_nsp_gemnoc
	},
	{
		&master_qnm_cmpnoc0,
		&slave_qns_llcc
	},
};

static struct icb_route route_qxm_nsp_qns_llcc = {
	ICBID_MASTER_CDSP_PROC,
	ICBID_SLAVE_LLCC,
	2,
	route_qxm_nsp_qns_llcc_hops
};

static struct icb_pair route_qxm_qup3_ebi_hops[4] = {
	{
		&master_qxm_qup3,
		&slave_qns_a1noc_snoc
	},
	{
		&master_qnm_aggre1_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qxm_qup3_ebi = {
	ICBID_MASTER_QUP_3,
	ICBID_SLAVE_EBI1,
	4,
	route_qxm_qup3_ebi_hops
};

static struct icb_pair route_qxm_qup3_qns_llcc_hops[3] = {
	{
		&master_qxm_qup3,
		&slave_qns_a1noc_snoc
	},
	{
		&master_qnm_aggre1_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qxm_qup3_qns_llcc = {
	ICBID_MASTER_QUP_3,
	ICBID_SLAVE_LLCC,
	3,
	route_qxm_qup3_qns_llcc_hops
};

static struct icb_pair route_xm_emac_0_ebi_hops[4] = {
	{
		&master_xm_emac_0,
		&slave_qns_a1noc_snoc
	},
	{
		&master_qnm_aggre1_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_xm_emac_0_ebi = {
	ICBID_MASTER_EMAC,
	ICBID_SLAVE_EBI1,
	4,
	route_xm_emac_0_ebi_hops
};

static struct icb_pair route_xm_emac_0_qns_llcc_hops[3] = {
	{
		&master_xm_emac_0,
		&slave_qns_a1noc_snoc
	},
	{
		&master_qnm_aggre1_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_xm_emac_0_qns_llcc = {
	ICBID_MASTER_EMAC,
	ICBID_SLAVE_LLCC,
	3,
	route_xm_emac_0_qns_llcc_hops
};

static struct icb_pair route_xm_emac_1_ebi_hops[4] = {
	{
		&master_xm_emac_1,
		&slave_qns_a1noc_snoc
	},
	{
		&master_qnm_aggre1_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_xm_emac_1_ebi = {
	ICBID_MASTER_EMAC_1,
	ICBID_SLAVE_EBI1,
	4,
	route_xm_emac_1_ebi_hops
};

static struct icb_pair route_xm_emac_1_qns_llcc_hops[3] = {
	{
		&master_xm_emac_1,
		&slave_qns_a1noc_snoc
	},
	{
		&master_qnm_aggre1_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_xm_emac_1_qns_llcc = {
	ICBID_MASTER_EMAC_1,
	ICBID_SLAVE_LLCC,
	3,
	route_xm_emac_1_qns_llcc_hops
};

static struct icb_pair route_xm_pcie3_0_ebi_hops[3] = {
	{
		&master_xm_pcie3_0,
		&slave_qns_pcie_mem_noc
	},
	{
		&master_qnm_pcie,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_xm_pcie3_0_ebi = {
	ICBID_MASTER_PCIE_0,
	ICBID_SLAVE_EBI1,
	3,
	route_xm_pcie3_0_ebi_hops
};

static struct icb_pair route_xm_pcie3_0_qns_llcc_hops[2] = {
	{
		&master_xm_pcie3_0,
		&slave_qns_pcie_mem_noc
	},
	{
		&master_qnm_pcie,
		&slave_qns_llcc
	},
};

static struct icb_route route_xm_pcie3_0_qns_llcc = {
	ICBID_MASTER_PCIE_0,
	ICBID_SLAVE_LLCC,
	2,
	route_xm_pcie3_0_qns_llcc_hops
};

static struct icb_pair route_xm_pcie3_1_ebi_hops[3] = {
	{
		&master_xm_pcie3_1,
		&slave_qns_pcie_mem_noc
	},
	{
		&master_qnm_pcie,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_xm_pcie3_1_ebi = {
	ICBID_MASTER_PCIE_1,
	ICBID_SLAVE_EBI1,
	3,
	route_xm_pcie3_1_ebi_hops
};

static struct icb_pair route_xm_pcie3_1_qns_llcc_hops[2] = {
	{
		&master_xm_pcie3_1,
		&slave_qns_pcie_mem_noc
	},
	{
		&master_qnm_pcie,
		&slave_qns_llcc
	},
};

static struct icb_route route_xm_pcie3_1_qns_llcc = {
	ICBID_MASTER_PCIE_1,
	ICBID_SLAVE_LLCC,
	2,
	route_xm_pcie3_1_qns_llcc_hops
};

/* NODE AND ROUTE LISTS */
static struct icb_route *icb_route_list[62] = {
	&route_chm_apps_ebi,
	&route_chm_apps_qhs_crypto0_cfg,
	&route_chm_apps_qhs_hwkm,
	&route_chm_apps_qhs_pke_wrapper_cfg,
	&route_chm_apps_qhs_qup0,
	&route_chm_apps_qhs_qup1,
	&route_chm_apps_qhs_qup2,
	&route_chm_apps_qhs_qup3,
	&route_chm_apps_qns_llcc,
	&route_hkwm_core_master_hkwm_core_slave,
	&route_ipa_core_master_ipa_core_slave,
	&route_llcc_mc_ebi,
	&route_pke_core_master_pke_core_slave,
	&route_qhm_qup0_ebi,
	&route_qhm_qup0_qns_llcc,
	&route_qhm_qup1_ebi,
	&route_qhm_qup1_qns_llcc,
	&route_qhm_qup2_ebi,
	&route_qhm_qup2_qns_llcc,
	&route_qnm_mdp0_0_ebi,
	&route_qnm_mdp0_0_qns_llcc,
	&route_qnm_mdp0_0_qns_mem_noc_hf,
	&route_qup0_core_master_qup0_core_slave,
	&route_qup1_core_master_qup1_core_slave,
	&route_qup2_core_master_qup2_core_slave,
	&route_qup3_core_master_qup3_core_slave,
	&route_qxm_crypto_0_ebi,
	&route_qxm_crypto_0_qns_llcc,
	&route_qxm_crypto_1_ebi,
	&route_qxm_crypto_1_qns_llcc,
	&route_qxm_dsp0_ebi,
	&route_qxm_dsp0_qhs_crypto0_cfg,
	&route_qxm_dsp0_qhs_hwkm,
	&route_qxm_dsp0_qhs_pke_wrapper_cfg,
	&route_qxm_dsp0_qhs_qup0,
	&route_qxm_dsp0_qhs_qup1,
	&route_qxm_dsp0_qhs_qup2,
	&route_qxm_dsp0_qhs_qup3,
	&route_qxm_dsp0_qns_llcc,
	&route_qxm_dsp1_ebi,
	&route_qxm_dsp1_qhs_crypto0_cfg,
	&route_qxm_dsp1_qhs_hwkm,
	&route_qxm_dsp1_qhs_pke_wrapper_cfg,
	&route_qxm_dsp1_qhs_qup0,
	&route_qxm_dsp1_qhs_qup1,
	&route_qxm_dsp1_qhs_qup2,
	&route_qxm_dsp1_qhs_qup3,
	&route_qxm_dsp1_qns_llcc,
	&route_qxm_nspb_ebi,
	&route_qxm_nspb_qns_llcc,
	&route_qxm_nsp_ebi,
	&route_qxm_nsp_qns_llcc,
	&route_qxm_qup3_ebi,
	&route_qxm_qup3_qns_llcc,
	&route_xm_emac_0_ebi,
	&route_xm_emac_0_qns_llcc,
	&route_xm_emac_1_ebi,
	&route_xm_emac_1_qns_llcc,
	&route_xm_pcie3_0_ebi,
	&route_xm_pcie3_0_qns_llcc,
	&route_xm_pcie3_1_ebi,
	&route_xm_pcie3_1_qns_llcc,
};

static struct icb_master *icb_master_list[33] = {
	&master_chm_apps,
	&master_hkwm_core_master,
	&master_ipa_core_master,
	&master_llcc_mc,
	&master_pke_core_master,
	&master_qhm_qup0,
	&master_qhm_qup1,
	&master_qhm_qup2,
	&master_qnm_aggre1_noc,
	&master_qnm_aggre2_noc,
	&master_qnm_cmpnoc0,
	&master_qnm_cmpnoc1,
	&master_qnm_gemnoc_cnoc,
	&master_qnm_gpdsp_sail,
	&master_qnm_mdp0_0,
	&master_qnm_mnoc_hf,
	&master_qnm_pcie,
	&master_qnm_snoc_sf,
	&master_qup0_core_master,
	&master_qup1_core_master,
	&master_qup2_core_master,
	&master_qup3_core_master,
	&master_qxm_crypto_0,
	&master_qxm_crypto_1,
	&master_qxm_dsp0,
	&master_qxm_dsp1,
	&master_qxm_nsp,
	&master_qxm_nspb,
	&master_qxm_qup3,
	&master_xm_emac_0,
	&master_xm_emac_1,
	&master_xm_pcie3_0,
	&master_xm_pcie3_1,
};

static struct icb_slave *icb_slave_list[25] = {
	&slave_ebi,
	&slave_hkwm_core_slave,
	&slave_ipa_core_slave,
	&slave_pke_core_slave,
	&slave_qhs_crypto0_cfg,
	&slave_qhs_hwkm,
	&slave_qhs_pke_wrapper_cfg,
	&slave_qhs_qup0,
	&slave_qhs_qup1,
	&slave_qhs_qup2,
	&slave_qhs_qup3,
	&slave_qns_a1noc_snoc,
	&slave_qns_a2noc_snoc,
	&slave_qns_gem_noc_cnoc,
	&slave_qns_gemnoc_sf,
	&slave_qns_gp_dsp_sail_noc,
	&slave_qns_llcc,
	&slave_qns_mem_noc_hf,
	&slave_qns_nsp_gemnoc,
	&slave_qns_nspb_gemnoc,
	&slave_qns_pcie_mem_noc,
	&slave_qup0_core_slave,
	&slave_qup1_core_slave,
	&slave_qup2_core_slave,
	&slave_qup3_core_slave,
};

static struct icb_hw_node *icb_hw_node_list[25] = {
	&bcm_acv,
	&bcm_ce0,
	&bcm_cn0,
	&bcm_cn1,
	&bcm_cn2,
	&bcm_gna0,
	&bcm_gnb0,
	&bcm_hk0,
	&bcm_ip0,
	&bcm_mc0,
	&bcm_mm0,
	&bcm_nsa0,
	&bcm_nsa1,
	&bcm_nsb0,
	&bcm_nsb1,
	&bcm_pci0,
	&bcm_pka0,
	&bcm_qup0,
	&bcm_qup1,
	&bcm_qup2,
	&bcm_sh0,
	&bcm_sh2,
	&bcm_sn0,
	&bcm_sn3,
	&bcm_sn4,
};

/* EXTERNAL DATA DECLARATIONS */
struct icb_info info = {
	62,
	icb_route_list,
	33,
	icb_master_list,
	25,
	icb_slave_list,
	25,
	icb_hw_node_list
};
