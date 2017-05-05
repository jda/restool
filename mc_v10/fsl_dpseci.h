/* Copyright 2013-2017 Freescale Semiconductor Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of the above-listed copyright holders nor the
 * names of any contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __FSL_DPSECI_V10_H
#define __FSL_DPSECI_V10_H

#include "../mc_v9/fsl_dpseci.h"

/* Data Path SEC Interface API
 * Contains initialization APIs and runtime control APIs for DPSECI
 */

struct fsl_mc_io;

/**
 * Enable the Order Restoration support
 */
#define DPSECI_OPT_HAS_OPR				0x000040

/**
 * Order Point Records are shared for the entire DPSECI
 */
#define DPSECI_OPT_OPR_SHARED				0x000080

/**
 * struct dpseci_cfg_v10 - Structure representing DPSECI configuration
 * @options:	Any combination of the following options:
 *		DPSECI_OPT_HAS_OPR
 *		DPSECI_OPT_OPR_SHARED
 * @num_tx_queues: Num of queues towards the SEC
 * @num_rx_queues: Num of queues back from the SEC
 * @priorities: Priorities for the SEC hardware processing;
 *		each place in the array is the priority of the tx queue
 *		towards the SEC,
 *		valid priorities are configured with values 1-8;
 */
struct dpseci_cfg_v10 {
	uint32_t options;
	uint8_t num_tx_queues;
	uint8_t num_rx_queues;
	uint8_t priorities[DPSECI_PRIO_NUM];
};

/**
 * dpseci_create_v10() - Create the DPSECI object
 * @mc_io:	Pointer to MC portal's I/O object
 * @dprc_token:	Parent container token; '0' for default container
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @cfg:	Configuration structure
 * @obj_id:	Returned object id
 *
 * Create the DPSECI object, allocate required resources and
 * perform required initialization.
 *
 * The object can be created either by declaring it in the
 * DPL file, or by calling this function.
 *
 * The function accepts an authentication token of a parent
 * container that this object should be assigned to. The token
 * can be '0' so the object will be assigned to the default container.
 * The newly created object can be opened with the returned
 * object id and using the container's associated tokens and MC portals.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpseci_create_v10_0(struct fsl_mc_io *mc_io,
			uint16_t dprc_token,
			uint32_t cmd_flags,
			const struct dpseci_cfg_v10 *cfg,
			uint32_t *obj_id);

int dpseci_create_v10_1(struct fsl_mc_io *mc_io,
			uint16_t dprc_token,
			uint32_t cmd_flags,
			const struct dpseci_cfg_v10 *cfg,
			uint32_t *obj_id);

/**
 * dpseci_destroy() - Destroy the DPSECI object and release all its resources.
 * @mc_io:	Pointer to MC portal's I/O object
 * @dprc_token: Parent container token; '0' for default container
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @object_id:	The object id; it must be a valid id within the container that
 * created this object;
 *
 * The function accepts the authentication token of the parent container that
 * created the object (not the one that currently owns the object). The object
 * is searched within parent using the provided 'object_id'.
 * All tokens to the object must be closed before calling destroy.
 *
 * Return:	'0' on Success; error code otherwise.
 */
int dpseci_destroy_v10(struct fsl_mc_io *mc_io,
		       uint16_t dprc_token,
		       uint32_t cmd_flags,
		       uint32_t object_id);

/**
 * struct dpseci_attr - Structure representing DPSECI attributes
 * @id:			DPSECI object ID
 * @version:		DPSECI version
 * @num_tx_queues:	Number of queues towards the SEC
 * @num_rx_queues:	Number of queues back from the SEC
 */
struct dpseci_attr_v10 {
	int id;
	uint8_t num_tx_queues;
	uint8_t num_rx_queues;
};

/**
 * dpseci_get_attributes() - Retrieve DPSECI attributes.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPSECI object
 * @attr:	Returned object's attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpseci_get_attributes_v10(struct fsl_mc_io *mc_io,
			  uint32_t cmd_flags,
			  uint16_t token,
			  struct dpseci_attr_v10 *attr);

/**
 * dpseci_get_version() - Get Data Path SEC Interface version
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @majorVer:	Major version of data path sec object
 * @minorVer:	Minor version of data path sec object
 *
 * Return:  '0' on Success; Error code otherwise.
 */
int dpseci_get_version_v10(struct fsl_mc_io *mc_io,
			   uint32_t cmd_flags,
			   uint16_t *majorVer,
			   uint16_t *minorVer);

#endif /* __FSL_DPSECI_H */
