// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * QTI ICB (Interconnect Bus) driver - Cacao target data.
 */

#include "icbuarbi.h"

/* HW NODE DECLARATIONS */
/* ACV */
static struct icb_hw_request bcm_acv_requests[1];
static struct icb_hw_node bcm_acv = {
	NULL, /**< node linkage */
	"ACV", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_acv_requests}, /**< request_list */
	0x0, /**< output */
};

/* CN0 */
static struct icb_hw_request bcm_cn0_requests[13];
static struct icb_hw_node bcm_cn0 = {
	NULL, /**< node linkage */
	"CN0", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 13, bcm_cn0_requests}, /**< request_list */
	0x1, /**< output */
};

/* CN1 */
static struct icb_hw_request bcm_cn1_requests[2];
static struct icb_hw_node bcm_cn1 = {
	NULL, /**< node linkage */
	"CN1", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_cn1_requests}, /**< request_list */
	0, /**< output */
};

/* LP0 */
static struct icb_hw_request bcm_lp0_requests[2];
static struct icb_hw_node bcm_lp0 = {
	NULL, /**< node linkage */
	"LP0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_lp0_requests}, /**< request_list */
	0, /**< output */
};

/* MC0 */
static struct icb_hw_request bcm_mc0_requests[1];
static struct icb_hw_node bcm_mc0 = {
	NULL, /**< node linkage */
	"MC0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_mc0_requests}, /**< request_list */
	0, /**< output */
};

/* MC1 */
static struct icb_hw_request bcm_mc1_requests[1];
static struct icb_hw_node bcm_mc1 = {
	NULL, /**< node linkage */
	"MC1", /**< name */
	ICB_HW_NODE_KIND_LATENCY, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_mc1_requests}, /**< request_list */
	0, /**< output */
};

/* MM0 */
static struct icb_hw_request bcm_mm0_requests[1];
static struct icb_hw_node bcm_mm0 = {
	NULL, /**< node linkage */
	"MM0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_mm0_requests}, /**< request_list */
	0, /**< output */
};

/* QUP0 */
static struct icb_hw_request bcm_qup0_requests[1];
static struct icb_hw_node bcm_qup0 = {
	NULL, /**< node linkage */
	"QUP0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_qup0_requests}, /**< request_list */
	0, /**< output */
};

/* QUP1 */
static struct icb_hw_request bcm_qup1_requests[1];
static struct icb_hw_node bcm_qup1 = {
	NULL, /**< node linkage */
	"QUP1", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_qup1_requests}, /**< request_list */
	0, /**< output */
};

/* SH0 */
static struct icb_hw_request bcm_sh0_requests[1];
static struct icb_hw_node bcm_sh0 = {
	NULL, /**< node linkage */
	"SH0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_sh0_requests}, /**< request_list */
	0, /**< output */
};

/* SH1 */
static struct icb_hw_request bcm_sh1_requests[5];
static struct icb_hw_node bcm_sh1 = {
	NULL, /**< node linkage */
	"SH1", /**< name */
	ICB_HW_NODE_KIND_VECTOR, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 5, bcm_sh1_requests}, /**< request_list */
	0x1, /**< output */
};

/* SH2 */
static struct icb_hw_request bcm_sh2_requests[1];
static struct icb_hw_node bcm_sh2 = {
	NULL, /**< node linkage */
	"SH2", /**< name */
	ICB_HW_NODE_KIND_LATENCY, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_sh2_requests}, /**< request_list */
	0, /**< output */
};

/* SN0 */
static struct icb_hw_request bcm_sn0_requests[1];
static struct icb_hw_node bcm_sn0 = {
	NULL, /**< node linkage */
	"SN0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_sn0_requests}, /**< request_list */
	0, /**< output */
};

/* SN2 */
static struct icb_hw_request bcm_sn2_requests[1];
static struct icb_hw_node bcm_sn2 = {
	NULL, /**< node linkage */
	"SN2", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 1, bcm_sn2_requests}, /**< request_list */
	0, /**< output */
};

/* WD0 */
static struct icb_hw_request bcm_wd0_requests[2];
static struct icb_hw_node bcm_wd0 = {
	NULL, /**< node linkage */
	"WD0", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_wd0_requests}, /**< request_list */
	0, /**< output */
};

/* MASTER DECLARATIONS */
/* CHM_APPS */
static struct icb_hw_node *master_chm_apps_hw_nodes[] = {
	&bcm_sh1,
};

static struct icb_master master_chm_apps = {
	ICBID_MASTER_APPSS_PROC,
	32, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_chm_apps_hw_nodes, /**< hw nodes */
};

/* LLCC_MC */
static struct icb_master master_llcc_mc = {
	ICBID_MASTER_LLCC,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QHM_QSPI */
static struct icb_master master_qhm_qspi = {
	ICBID_MASTER_QSPI_0,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QHM_QUP0 */
static struct icb_master master_qhm_qup0 = {
	ICBID_MASTER_QUP_0,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QHM_QUP1 */
static struct icb_master master_qhm_qup1 = {
	ICBID_MASTER_QUP_1,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QNM_AGGRE_NOC */
static struct icb_hw_node *master_qnm_aggre_noc_hw_nodes[] = {
	&bcm_sn2,
};

static struct icb_master master_qnm_aggre_noc = {
	ICBID_MASTER_ANOC_SNOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_aggre_noc_hw_nodes, /**< hw nodes */
};

/* QNM_GEMNOC_CNOC */
static struct icb_hw_node *master_qnm_gemnoc_cnoc_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_master master_qnm_gemnoc_cnoc = {
	ICBID_MASTER_GEM_NOC_CNOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_gemnoc_cnoc_hw_nodes, /**< hw nodes */
};

/* QNM_LPASS_GEMNOC */
static struct icb_hw_node *master_qnm_lpass_gemnoc_hw_nodes[] = {
	&bcm_sh1,
};

static struct icb_master master_qnm_lpass_gemnoc = {
	ICBID_MASTER_LPASS_GEM_NOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_lpass_gemnoc_hw_nodes, /**< hw nodes */
};

/* QNM_LPASS_LPINOC */
static struct icb_hw_node *master_qnm_lpass_lpinoc_hw_nodes[] = {
	&bcm_lp0,
};

static struct icb_master master_qnm_lpass_lpinoc = {
	ICBID_MASTER_LPASS_LPINOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_lpass_lpinoc_hw_nodes, /**< hw nodes */
};

/* QNM_LPIAON_NOC */
static struct icb_master master_qnm_lpiaon_noc = {
	ICBID_MASTER_LPIAON_NOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QNM_LPINOC_DSP_QNS4M */
static struct icb_master master_qnm_lpinoc_dsp_qns4m = {
	ICBID_MASTER_LPASS_PROC,
	32, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QNM_MDSS */
static struct icb_hw_node *master_qnm_mdss_hw_nodes[] = {
	&bcm_wd0,
};

static struct icb_master master_qnm_mdss = {
	ICBID_MASTER_MDSS,
	32, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_mdss_hw_nodes, /**< hw nodes */
};

/* QNM_MNOC_HF */
static struct icb_hw_node *master_qnm_mnoc_hf_hw_nodes[] = {
	&bcm_sh1,
};

static struct icb_master master_qnm_mnoc_hf = {
	ICBID_MASTER_MNOC_HF_MEM_NOC,
	32, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_mnoc_hf_hw_nodes, /**< hw nodes */
};

/* QNM_SNOC_SF */
static struct icb_hw_node *master_qnm_snoc_sf_hw_nodes[] = {
	&bcm_sh1,
};

static struct icb_master master_qnm_snoc_sf = {
	ICBID_MASTER_SNOC_SF_MEM_NOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qnm_snoc_sf_hw_nodes, /**< hw nodes */
};

/* QNM_WDSS_HF */
static struct icb_master master_qnm_wdss_hf = {
	ICBID_MASTER_WDSS_HF,
	32, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QSM_CFG */
static struct icb_hw_node *master_qsm_cfg_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_master master_qsm_cfg = {
	ICBID_MASTER_CNOC_CFG,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	master_qsm_cfg_hw_nodes, /**< hw nodes */
};

/* QUP0_CORE_MASTER */
static struct icb_master master_qup0_core_master = {
	ICBID_MASTER_QUP_CORE_0,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QUP1_CORE_MASTER */
static struct icb_master master_qup1_core_master = {
	ICBID_MASTER_QUP_CORE_1,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* SLAVE DECLARATIONS */
/* EBI */
static struct icb_hw_node *slave_ebi_hw_nodes[] = {
	&bcm_mc0,
	&bcm_acv,
};

static struct icb_slave slave_ebi = {
	ICBID_SLAVE_EBI1,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	2, /**< number of hw_nodes */
	slave_ebi_hw_nodes, /**< hw nodes */
};

/* QHS_CLK_CTL */
static struct icb_hw_node *slave_qhs_clk_ctl_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qhs_clk_ctl = {
	ICBID_SLAVE_CLK_CTL,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qhs_clk_ctl_hw_nodes, /**< hw nodes */
};

/* QHS_IMEM_CFG */
static struct icb_hw_node *slave_qhs_imem_cfg_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qhs_imem_cfg = {
	ICBID_SLAVE_IMEM_CFG,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qhs_imem_cfg_hw_nodes, /**< hw nodes */
};

/* QHS_QRNG */
static struct icb_hw_node *slave_qhs_qrng_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qhs_qrng = {
	ICBID_SLAVE_QRNG,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qhs_qrng_hw_nodes, /**< hw nodes */
};

/* QHS_QUP0 */
static struct icb_hw_node *slave_qhs_qup0_hw_nodes[] = {
	&bcm_cn1,
};

static struct icb_slave slave_qhs_qup0 = {
	ICBID_SLAVE_QUP_0,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qhs_qup0_hw_nodes, /**< hw nodes */
};

/* QHS_QUP1 */
static struct icb_hw_node *slave_qhs_qup1_hw_nodes[] = {
	&bcm_cn1,
};

static struct icb_slave slave_qhs_qup1 = {
	ICBID_SLAVE_QUP_1,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qhs_qup1_hw_nodes, /**< hw nodes */
};

/* QHS_TCSR */
static struct icb_hw_node *slave_qhs_tcsr_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qhs_tcsr = {
	ICBID_SLAVE_TCSR,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qhs_tcsr_hw_nodes, /**< hw nodes */
};

/* QHS_TLMM */
static struct icb_hw_node *slave_qhs_tlmm_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qhs_tlmm = {
	ICBID_SLAVE_TLMM,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qhs_tlmm_hw_nodes, /**< hw nodes */
};

/* QNS_ANOC_SNOC */
static struct icb_slave slave_qns_anoc_snoc = {
	ICBID_SLAVE_ANOC_SNOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QNS_DPU_HF_MMNOC */
static struct icb_hw_node *slave_qns_dpu_hf_mmnoc_hw_nodes[] = {
	&bcm_wd0,
};

static struct icb_slave slave_qns_dpu_hf_mmnoc = {
	ICBID_SLAVE_DPU_HF_MMNOC,
	32, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_dpu_hf_mmnoc_hw_nodes, /**< hw nodes */
};

/* QNS_GEM_NOC_CNOC */
static struct icb_hw_node *slave_qns_gem_noc_cnoc_hw_nodes[] = {
	&bcm_sh1,
};

static struct icb_slave slave_qns_gem_noc_cnoc = {
	ICBID_SLAVE_GEM_NOC_CNOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_gem_noc_cnoc_hw_nodes, /**< hw nodes */
};

/* QNS_GEMNOC_SF */
static struct icb_hw_node *slave_qns_gemnoc_sf_hw_nodes[] = {
	&bcm_sn0,
};

static struct icb_slave slave_qns_gemnoc_sf = {
	ICBID_SLAVE_SNOC_GEM_NOC_SF,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_gemnoc_sf_hw_nodes, /**< hw nodes */
};

/* QNS_LLCC */
static struct icb_hw_node *slave_qns_llcc_hw_nodes[] = {
	&bcm_sh0,
};

static struct icb_slave slave_qns_llcc = {
	ICBID_SLAVE_LLCC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_llcc_hw_nodes, /**< hw nodes */
};

/* QNS_LPASS_AG_NOC_GEMNOC */
static struct icb_slave slave_qns_lpass_ag_noc_gemnoc = {
	ICBID_SLAVE_LPASS_GEM_NOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QNS_LPASS_AGGNOC */
static struct icb_hw_node *slave_qns_lpass_aggnoc_hw_nodes[] = {
	&bcm_lp0,
};

static struct icb_slave slave_qns_lpass_aggnoc = {
	ICBID_SLAVE_LPIAON_NOC_LPASS_AG_NOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_lpass_aggnoc_hw_nodes, /**< hw nodes */
};

/* QNS_LPI_AON_NOC */
static struct icb_slave slave_qns_lpi_aon_noc = {
	ICBID_SLAVE_LPICX_NOC_LPIAON_NOC,
	16, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* QNS_MEM_NOC_HF */
static struct icb_hw_node *slave_qns_mem_noc_hf_hw_nodes[] = {
	&bcm_mm0,
};

static struct icb_slave slave_qns_mem_noc_hf = {
	ICBID_SLAVE_MNOC_HF_MEM_NOC,
	32, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qns_mem_noc_hf_hw_nodes, /**< hw nodes */
};

/* QSS_CFG */
static struct icb_hw_node *slave_qss_cfg_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qss_cfg = {
	ICBID_SLAVE_CNOC_CFG,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qss_cfg_hw_nodes, /**< hw nodes */
};

/* QSS_DDRSS_CFG */
static struct icb_hw_node *slave_qss_ddrss_cfg_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qss_ddrss_cfg = {
	ICBID_SLAVE_DDRSS_CFG,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qss_ddrss_cfg_hw_nodes, /**< hw nodes */
};

/* QSS_LPASS_QTB_CFG */
static struct icb_hw_node *slave_qss_lpass_qtb_cfg_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qss_lpass_qtb_cfg = {
	ICBID_SLAVE_LPASS_QTB_CFG,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qss_lpass_qtb_cfg_hw_nodes, /**< hw nodes */
};

/* QUP0_CORE_SLAVE */
static struct icb_hw_node *slave_qup0_core_slave_hw_nodes[] = {
	&bcm_qup0,
};

static struct icb_slave slave_qup0_core_slave = {
	ICBID_SLAVE_QUP_CORE_0,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qup0_core_slave_hw_nodes, /**< hw nodes */
};

/* QUP1_CORE_SLAVE */
static struct icb_hw_node *slave_qup1_core_slave_hw_nodes[] = {
	&bcm_qup1,
};

static struct icb_slave slave_qup1_core_slave = {
	ICBID_SLAVE_QUP_CORE_1,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qup1_core_slave_hw_nodes, /**< hw nodes */
};

/* QXS_IMEM */
static struct icb_hw_node *slave_qxs_imem_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_qxs_imem = {
	ICBID_SLAVE_IMEM,
	8, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qxs_imem_hw_nodes, /**< hw nodes */
};

/* XS_QDSS_STM */
static struct icb_hw_node *slave_xs_qdss_stm_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_xs_qdss_stm = {
	ICBID_SLAVE_QDSS_STM,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_xs_qdss_stm_hw_nodes, /**< hw nodes */
};

/* XS_SYS_TCU_CFG */
static struct icb_hw_node *slave_xs_sys_tcu_cfg_hw_nodes[] = {
	&bcm_cn0,
};

static struct icb_slave slave_xs_sys_tcu_cfg = {
	ICBID_SLAVE_TCU,
	8, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_xs_sys_tcu_cfg_hw_nodes, /**< hw nodes */
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

static struct icb_pair route_chm_apps_qhs_clk_ctl_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_qhs_clk_ctl
	},
};

static struct icb_route route_chm_apps_qhs_clk_ctl = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_CLK_CTL,
	3,
	route_chm_apps_qhs_clk_ctl_hops
};

static struct icb_pair route_chm_apps_qhs_imem_cfg_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_qhs_imem_cfg
	},
};

static struct icb_route route_chm_apps_qhs_imem_cfg = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_IMEM_CFG,
	3,
	route_chm_apps_qhs_imem_cfg_hops
};

static struct icb_pair route_chm_apps_qhs_qrng_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qhs_qrng
	},
};

static struct icb_route route_chm_apps_qhs_qrng = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QRNG,
	2,
	route_chm_apps_qhs_qrng_hops
};

static struct icb_pair route_chm_apps_qhs_qup0_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_qhs_qup0
	},
};

static struct icb_route route_chm_apps_qhs_qup0 = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QUP_0,
	3,
	route_chm_apps_qhs_qup0_hops
};

static struct icb_pair route_chm_apps_qhs_qup1_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_qhs_qup1
	},
};

static struct icb_route route_chm_apps_qhs_qup1 = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QUP_1,
	3,
	route_chm_apps_qhs_qup1_hops
};

static struct icb_pair route_chm_apps_qhs_tcsr_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_qhs_tcsr
	},
};

static struct icb_route route_chm_apps_qhs_tcsr = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_TCSR,
	3,
	route_chm_apps_qhs_tcsr_hops
};

static struct icb_pair route_chm_apps_qhs_tlmm_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_qhs_tlmm
	},
};

static struct icb_route route_chm_apps_qhs_tlmm = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_TLMM,
	3,
	route_chm_apps_qhs_tlmm_hops
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

static struct icb_pair route_chm_apps_qss_ddrss_cfg_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_ddrss_cfg
	},
};

static struct icb_route route_chm_apps_qss_ddrss_cfg = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_DDRSS_CFG,
	2,
	route_chm_apps_qss_ddrss_cfg_hops
};

static struct icb_pair route_chm_apps_qss_lpass_qtb_cfg_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_qss_lpass_qtb_cfg
	},
};

static struct icb_route route_chm_apps_qss_lpass_qtb_cfg = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_LPASS_QTB_CFG,
	3,
	route_chm_apps_qss_lpass_qtb_cfg_hops
};

static struct icb_pair route_chm_apps_qxs_imem_hops[2] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qxs_imem
	},
};

static struct icb_route route_chm_apps_qxs_imem = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_IMEM,
	2,
	route_chm_apps_qxs_imem_hops
};

static struct icb_pair route_chm_apps_xs_qdss_stm_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_xs_qdss_stm
	},
};

static struct icb_route route_chm_apps_xs_qdss_stm = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_QDSS_STM,
	3,
	route_chm_apps_xs_qdss_stm_hops
};

static struct icb_pair route_chm_apps_xs_sys_tcu_cfg_hops[3] = {
	{
		&master_chm_apps,
		&slave_qns_gem_noc_cnoc
	},
	{
		&master_qnm_gemnoc_cnoc,
		&slave_qss_cfg
	},
	{
		&master_qsm_cfg,
		&slave_xs_sys_tcu_cfg
	},
};

static struct icb_route route_chm_apps_xs_sys_tcu_cfg = {
	ICBID_MASTER_APPSS_PROC,
	ICBID_SLAVE_TCU,
	3,
	route_chm_apps_xs_sys_tcu_cfg_hops
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

static struct icb_pair route_qhm_qspi_ebi_hops[4] = {
	{
		&master_qhm_qspi,
		&slave_qns_anoc_snoc
	},
	{
		&master_qnm_aggre_noc,
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

static struct icb_route route_qhm_qspi_ebi = {
	ICBID_MASTER_QSPI_0,
	ICBID_SLAVE_EBI1,
	4,
	route_qhm_qspi_ebi_hops
};

static struct icb_pair route_qhm_qspi_qns_llcc_hops[3] = {
	{
		&master_qhm_qspi,
		&slave_qns_anoc_snoc
	},
	{
		&master_qnm_aggre_noc,
		&slave_qns_gemnoc_sf
	},
	{
		&master_qnm_snoc_sf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qhm_qspi_qns_llcc = {
	ICBID_MASTER_QSPI_0,
	ICBID_SLAVE_LLCC,
	3,
	route_qhm_qspi_qns_llcc_hops
};

static struct icb_pair route_qhm_qup0_ebi_hops[4] = {
	{
		&master_qhm_qup0,
		&slave_qns_anoc_snoc
	},
	{
		&master_qnm_aggre_noc,
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
		&slave_qns_anoc_snoc
	},
	{
		&master_qnm_aggre_noc,
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
		&slave_qns_anoc_snoc
	},
	{
		&master_qnm_aggre_noc,
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
		&slave_qns_anoc_snoc
	},
	{
		&master_qnm_aggre_noc,
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

static struct icb_pair route_qnm_lpinoc_dsp_qns4m_ebi_hops[5] = {
	{
		&master_qnm_lpinoc_dsp_qns4m,
		&slave_qns_lpi_aon_noc
	},
	{
		&master_qnm_lpass_lpinoc,
		&slave_qns_lpass_aggnoc
	},
	{
		&master_qnm_lpiaon_noc,
		&slave_qns_lpass_ag_noc_gemnoc
	},
	{
		&master_qnm_lpass_gemnoc,
		&slave_qns_llcc
	},
	{
		&master_llcc_mc,
		&slave_ebi
	},
};

static struct icb_route route_qnm_lpinoc_dsp_qns4m_ebi = {
	ICBID_MASTER_LPASS_PROC,
	ICBID_SLAVE_EBI1,
	5,
	route_qnm_lpinoc_dsp_qns4m_ebi_hops
};

static struct icb_pair route_qnm_lpinoc_dsp_qns4m_qns_llcc_hops[4] = {
	{
		&master_qnm_lpinoc_dsp_qns4m,
		&slave_qns_lpi_aon_noc
	},
	{
		&master_qnm_lpass_lpinoc,
		&slave_qns_lpass_aggnoc
	},
	{
		&master_qnm_lpiaon_noc,
		&slave_qns_lpass_ag_noc_gemnoc
	},
	{
		&master_qnm_lpass_gemnoc,
		&slave_qns_llcc
	},
};

static struct icb_route route_qnm_lpinoc_dsp_qns4m_qns_llcc = {
	ICBID_MASTER_LPASS_PROC,
	ICBID_SLAVE_LLCC,
	4,
	route_qnm_lpinoc_dsp_qns4m_qns_llcc_hops
};

static struct icb_pair route_qnm_mdss_ebi_hops[4] = {
	{
		&master_qnm_mdss,
		&slave_qns_dpu_hf_mmnoc
	},
	{
		&master_qnm_wdss_hf,
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

static struct icb_route route_qnm_mdss_ebi = {
	ICBID_MASTER_MDSS,
	ICBID_SLAVE_EBI1,
	4,
	route_qnm_mdss_ebi_hops
};

static struct icb_pair route_qnm_mdss_qns_llcc_hops[3] = {
	{
		&master_qnm_mdss,
		&slave_qns_dpu_hf_mmnoc
	},
	{
		&master_qnm_wdss_hf,
		&slave_qns_mem_noc_hf
	},
	{
		&master_qnm_mnoc_hf,
		&slave_qns_llcc
	},
};

static struct icb_route route_qnm_mdss_qns_llcc = {
	ICBID_MASTER_MDSS,
	ICBID_SLAVE_LLCC,
	3,
	route_qnm_mdss_qns_llcc_hops
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

/* NODE AND ROUTE LISTS */
static struct icb_route *icb_route_list[27] = {
	&route_chm_apps_ebi,
	&route_chm_apps_qhs_clk_ctl,
	&route_chm_apps_qhs_imem_cfg,
	&route_chm_apps_qhs_qrng,
	&route_chm_apps_qhs_qup0,
	&route_chm_apps_qhs_qup1,
	&route_chm_apps_qhs_tcsr,
	&route_chm_apps_qhs_tlmm,
	&route_chm_apps_qns_llcc,
	&route_chm_apps_qss_ddrss_cfg,
	&route_chm_apps_qss_lpass_qtb_cfg,
	&route_chm_apps_qxs_imem,
	&route_chm_apps_xs_qdss_stm,
	&route_chm_apps_xs_sys_tcu_cfg,
	&route_llcc_mc_ebi,
	&route_qhm_qspi_ebi,
	&route_qhm_qspi_qns_llcc,
	&route_qhm_qup0_ebi,
	&route_qhm_qup0_qns_llcc,
	&route_qhm_qup1_ebi,
	&route_qhm_qup1_qns_llcc,
	&route_qnm_lpinoc_dsp_qns4m_ebi,
	&route_qnm_lpinoc_dsp_qns4m_qns_llcc,
	&route_qnm_mdss_ebi,
	&route_qnm_mdss_qns_llcc,
	&route_qup0_core_master_qup0_core_slave,
	&route_qup1_core_master_qup1_core_slave,
};

static struct icb_master *icb_master_list[18] = {
	&master_chm_apps,
	&master_llcc_mc,
	&master_qhm_qspi,
	&master_qhm_qup0,
	&master_qhm_qup1,
	&master_qnm_aggre_noc,
	&master_qnm_gemnoc_cnoc,
	&master_qnm_lpass_gemnoc,
	&master_qnm_lpass_lpinoc,
	&master_qnm_lpiaon_noc,
	&master_qnm_lpinoc_dsp_qns4m,
	&master_qnm_mdss,
	&master_qnm_mnoc_hf,
	&master_qnm_snoc_sf,
	&master_qnm_wdss_hf,
	&master_qsm_cfg,
	&master_qup0_core_master,
	&master_qup1_core_master,
};

static struct icb_slave *icb_slave_list[25] = {
	&slave_ebi,
	&slave_qhs_clk_ctl,
	&slave_qhs_imem_cfg,
	&slave_qhs_qrng,
	&slave_qhs_qup0,
	&slave_qhs_qup1,
	&slave_qhs_tcsr,
	&slave_qhs_tlmm,
	&slave_qns_anoc_snoc,
	&slave_qns_dpu_hf_mmnoc,
	&slave_qns_gem_noc_cnoc,
	&slave_qns_gemnoc_sf,
	&slave_qns_llcc,
	&slave_qns_lpass_ag_noc_gemnoc,
	&slave_qns_lpass_aggnoc,
	&slave_qns_lpi_aon_noc,
	&slave_qns_mem_noc_hf,
	&slave_qss_cfg,
	&slave_qss_ddrss_cfg,
	&slave_qss_lpass_qtb_cfg,
	&slave_qup0_core_slave,
	&slave_qup1_core_slave,
	&slave_qxs_imem,
	&slave_xs_qdss_stm,
	&slave_xs_sys_tcu_cfg,
};

static struct icb_hw_node *icb_hw_node_list[15] = {
	&bcm_acv,
	&bcm_cn0,
	&bcm_cn1,
	&bcm_lp0,
	&bcm_mc0,
	&bcm_mc1,
	&bcm_mm0,
	&bcm_qup0,
	&bcm_qup1,
	&bcm_sh0,
	&bcm_sh1,
	&bcm_sh2,
	&bcm_sn0,
	&bcm_sn2,
	&bcm_wd0,
};

/* EXTERNAL DATA DECLARATIONS */
struct icb_info info = {
	27, /**< number of routes */
	icb_route_list, /**< routes */
	18, /**< number of masters */
	icb_master_list, /**< masters */
	25, /**< number of slaves */
	icb_slave_list, /**< slaves */
	15, /**< number of hw_nodes */
	icb_hw_node_list, /**< hw_nodes */
};
