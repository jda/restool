/* Copyright 2014 Freescale Semiconductor Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "fsl_mc_sys.h"
#include "fsl_mc_cmd.h"
#include "fsl_dpio.h"
#include "fsl_dpio_cmd.h"

#define CMD_PREP(_param, _offset, _width, _type, _arg) \
	(cmd.params[_param] |= u64_enc((_offset), (_width), (_arg)))

#define RSP_READ(_param, _offset, _width, _type, _arg) \
	(*(_arg) = (_type)u64_dec(cmd.params[_param], (_offset), (_width)))

#define RSP_READ_STRUCT(_param, _offset, _width, _type, _arg) \
	(_arg = (_type)u64_dec(cmd.params[_param], (_offset), (_width)))

int dpio_create(struct fsl_mc_io *mc_io, const struct dpio_cfg *cfg,
		uint16_t *dpio_handle)
{
	struct mc_command cmd = { 0 };
	int err;

	cmd.header = mc_encode_cmd_header(MC_DPIO_CMDID_CREATE,
					  DPIO_CMDSZ_CREATE,
					  MC_CMD_PRI_LOW, 0);

	DPIO_CMD_CREATE(CMD_PREP);

	err = mc_send_command(mc_io, &cmd);
	if (!err)
		*dpio_handle = MC_CMD_HDR_READ_AUTHID(cmd.header);

	return err;
}

int dpio_open(struct fsl_mc_io *mc_io, int dpio_id, uint16_t *dpio_handle)
{
	struct mc_command cmd = { 0 };
	int err;

	cmd.header = mc_encode_cmd_header(MC_DPIO_CMDID_OPEN,
					  MC_CMD_OPEN_SIZE,
					  MC_CMD_PRI_LOW, 0);

	DPIO_CMD_OPEN(CMD_PREP);

	err = mc_send_command(mc_io, &cmd);
	if (!err)
		*dpio_handle = MC_CMD_HDR_READ_AUTHID(cmd.header);

	return err;
}

int dpio_close(struct fsl_mc_io *mc_io, uint16_t dpio_handle)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(MC_CMDID_CLOSE,
					  MC_CMD_CLOSE_SIZE,
					  MC_CMD_PRI_HIGH,
					  dpio_handle);

	return mc_send_command(mc_io, &cmd);
}

int dpio_destroy(struct fsl_mc_io *mc_io, uint16_t dpio_handle)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_DESTROY,
					  DPIO_CMDSZ_DESTROY,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	return mc_send_command(mc_io, &cmd);
}

int dpio_enable(struct fsl_mc_io *mc_io, uint16_t dpio_handle)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_ENABLE,
					  DPIO_CMDSZ_ENABLE,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	return mc_send_command(mc_io, &cmd);
}

int dpio_disable(struct fsl_mc_io *mc_io, uint16_t dpio_handle)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_DISABLE,
					  DPIO_CMDSZ_DISABLE,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	return mc_send_command(mc_io, &cmd);
}

int dpio_get_attributes(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
			struct dpio_attr *attr)
{
	int err;
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_GET_ATTR,
					  DPIO_CMDSZ_GET_ATTR,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	err = mc_send_command(mc_io, &cmd);
	if (!err)
		DPIO_RSP_GET_ATTR(RSP_READ_STRUCT);

	return err;
}

int dpio_get_irq(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
		 uint8_t irq_index,
		 int *type,
		 uint64_t *irq_paddr,
		 uint32_t *irq_val,
		 int *user_irq_id)
{
	int err;
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_GET_IRQ,
					  DPIO_CMDSZ_GET_IRQ,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_GET_IRQ(CMD_PREP);

	err = mc_send_command(mc_io, &cmd);
	if (!err)
		DPIO_RSP_GET_IRQ(RSP_READ);

	return err;
}

int dpio_set_irq(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
		 uint8_t irq_index,
		 uint64_t irq_paddr,
		 uint32_t irq_val,
		 int user_irq_id)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_SET_IRQ,
					  DPIO_CMDSZ_SET_IRQ,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_SET_IRQ(CMD_PREP);
	return mc_send_command(mc_io, &cmd);
}

int dpio_get_irq_enable(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
			uint8_t irq_index, uint8_t *enable_state)
{
	int err;
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_GET_IRQ_ENABLE,
					  DPIO_CMDSZ_GET_IRQ_ENABLE,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_GET_IRQ_ENABLE(CMD_PREP);
	err = mc_send_command(mc_io, &cmd);
	if (!err)
		DPIO_RSP_GET_IRQ_ENABLE(RSP_READ);

	return err;
}

int dpio_set_irq_enable(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
			uint8_t irq_index, uint8_t enable_state)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_SET_IRQ_ENABLE,
					  DPIO_CMDSZ_SET_IRQ_ENABLE,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_SET_IRQ_ENABLE(CMD_PREP);
	return mc_send_command(mc_io, &cmd);
}

int dpio_get_irq_mask(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
		      uint8_t irq_index, uint32_t *mask)
{
	int err;
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_GET_IRQ_MASK,
					  DPIO_CMDSZ_GET_IRQ_MASK,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_GET_IRQ_MASK(CMD_PREP);
	err = mc_send_command(mc_io, &cmd);
	if (!err)
		DPIO_RSP_GET_IRQ_MASK(RSP_READ);

	return err;
}

int dpio_set_irq_mask(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
		      uint8_t irq_index, uint32_t mask)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_SET_IRQ_MASK,
					  DPIO_CMDSZ_SET_IRQ_MASK,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_SET_IRQ_MASK(CMD_PREP);
	return mc_send_command(mc_io, &cmd);
}

int dpio_get_irq_status(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
			uint8_t irq_index, uint32_t *status)
{
	int err;
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_GET_IRQ_STATUS,
					  DPIO_CMDSZ_GET_IRQ_STATUS,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_GET_IRQ_STATUS(CMD_PREP);
	err = mc_send_command(mc_io, &cmd);
	if (!err)
		DPIO_RSP_GET_IRQ_STATUS(RSP_READ);

	return err;
}

int dpio_clear_irq_status(struct fsl_mc_io *mc_io, uint16_t dpio_handle,
			  uint8_t irq_index, uint32_t status)
{
	struct mc_command cmd = { 0 };

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_CLEAR_IRQ_STATUS,
					  DPIO_CMDSZ_CLEAR_IRQ_STATUS,
					  MC_CMD_PRI_LOW,
					  dpio_handle);

	DPIO_CMD_CLEAR_IRQ_STATUS(CMD_PREP);
	return mc_send_command(mc_io, &cmd);
}