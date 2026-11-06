// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * QTI ICB (Interconnect Bus) driver - Lemans target data.
 *
 * Trimmed to the single route an in-tree consumer (the GENI SPI driver,
 * #8042, dTPM on QUP2 SE2) actually votes. The full BCM/route catalog for
 * Lemans carried crypto/PKA/HWKM/IPA/LLCC/EBI and the other three QUP cores
 * with no landing consumer; add those back alongside their consumers.
 */

#include "icbuarbi.h"

/* HW NODE DECLARATIONS */
/* QUP2 */
static struct icb_hw_request bcm_qup2_requests[2];
static struct icb_hw_node bcm_qup2 = {
	NULL, /**< node linkage */
	"QUP2", /**< name */
	ICB_HW_NODE_KIND_BANDWIDTH, /**< type */
	0, /**< hw_id */
	0, /**< width */
	0, /**< bw_unit */
	0, /**< clk_id */
	false, /**< is_dirty */
	0, /**< vote */
	{0, 0}, /**< state */
	{0, 2, bcm_qup2_requests}, /**< request_list */
	0, /**< output */
};

/* MASTER DECLARATIONS */
/* QUP2_CORE_MASTER */
static struct icb_master master_qup2_core_master = {
	ICBID_MASTER_QUP_CORE_2,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	0, /**< number of hw_nodes */
	NULL, /**< hw nodes */
};

/* SLAVE DECLARATIONS */
/* QUP2_CORE_SLAVE */
static struct icb_hw_node *slave_qup2_core_slave_hw_nodes[] = {
	&bcm_qup2,
};

static struct icb_slave slave_qup2_core_slave = {
	ICBID_SLAVE_QUP_CORE_2,
	4, /**< width */
	1, /**< num ports */
	{0, 0}, /**< state */
	{0, 0, NULL}, /**< request_list */
	1, /**< number of hw_nodes */
	slave_qup2_core_slave_hw_nodes, /**< hw nodes */
};

/* ROUTE DECLARATIONS */
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

/* NODE AND ROUTE LISTS */
static struct icb_route *icb_route_list[1] = {
	&route_qup2_core_master_qup2_core_slave,
};

static struct icb_master *icb_master_list[1] = {
	&master_qup2_core_master,
};

static struct icb_slave *icb_slave_list[1] = {
	&slave_qup2_core_slave,
};

static struct icb_hw_node *icb_hw_node_list[1] = {
	&bcm_qup2,
};

/* EXTERNAL DATA DECLARATIONS */
struct icb_info info = {
	1, /**< number of routes */
	icb_route_list, /**< routes */
	1, /**< number of masters */
	icb_master_list, /**< masters */
	1, /**< number of slaves */
	icb_slave_list, /**< slaves */
	1, /**< number of hw_nodes */
	icb_hw_node_list, /**< hw_nodes */
};
