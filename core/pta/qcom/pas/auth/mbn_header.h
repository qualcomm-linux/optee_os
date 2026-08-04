/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * MBN boot-image header.
 *
 * Each PIL firmware image carries a hash-segment header describing the
 * signature, certificate chain, and metadata blobs that follow it in the
 * image. struct mbn_boot_image_header holds the fields common to every
 * header version; struct mbn_boot_image_header_v6 is the v6 layout,
 * identified by header_version == 6, which extends it with two metadata
 * size fields. The two structs share the layout of their common leading
 * fields, so a header can be read as the base struct first to check
 * header_version before being reinterpreted as the v6 struct.
 */

#ifndef MBN_HEADER_H
#define MBN_HEADER_H

#include <stdint.h>

#define MBN_HEADER_VERSION_6	6

struct mbn_boot_image_header {
	uint32_t reserved1;
	uint32_t header_version;
	uint32_t qc_signature_size;
	uint32_t qc_cert_chain_size;
	uint32_t image_size;
	uint32_t code_size;
	uint32_t reserved2;
	uint32_t oem_signature_size;
	uint32_t reserved3;
	uint32_t oem_cert_chain_size;
};

struct mbn_boot_image_header_v6 {
	uint32_t reserved1;
	uint32_t header_version;
	uint32_t qc_signature_size;
	uint32_t qc_cert_chain_size;
	uint32_t image_size;
	uint32_t code_size;
	uint32_t reserved2;
	uint32_t oem_signature_size;
	uint32_t reserved3;
	uint32_t oem_cert_chain_size;
	uint32_t qc_metadata_size;
	uint32_t oem_metadata_size;
};

#endif /* MBN_HEADER_H */

