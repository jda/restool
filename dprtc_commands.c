/*
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 * Author: Lijun Pan <Lijun.Pan@freescale.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation  and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of any
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include "restool.h"
#include "utils.h"
#include "mc_v9/fsl_dprtc.h"

enum mc_cmd_status mc_status;

/**
 * dprtc info command options
 */
enum dprtc_info_options {
	INFO_OPT_HELP = 0,
	INFO_OPT_VERBOSE,
};

static struct option dprtc_info_options[] = {
	[INFO_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	[INFO_OPT_VERBOSE] = {
		.name = "verbose",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dprtc_info_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

/**
 * dprtc create command options
 */
enum dprtc_create_options {
	CREATE_OPT_HELP = 0,
	CREATE_OPT_OPTIONS,
};

static struct option dprtc_create_options[] = {
	[CREATE_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_OPTIONS] = {
		.name = "options",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dprtc_create_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

/**
 * dprtc destroy command options
 */
enum dprtc_destroy_options {
	DESTROY_OPT_HELP = 0,
};

static struct option dprtc_destroy_options[] = {
	[DESTROY_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dprtc_destroy_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

static const struct flib_ops dprtc_ops = {
	.obj_open = dprtc_open,
	.obj_close = dprtc_close,
	.obj_get_irq_mask = dprtc_get_irq_mask,
	.obj_get_irq_status = dprtc_get_irq_status,
};

static int cmd_dprtc_help(void)
{
	static const char help_msg[] =
		"\n"
		"restool dprtc <command> [--help] [ARGS...]\n"
		"Where <command> can be:\n"
		"   info - displays detailed information about a DPRTC object.\n"
		"   create - creates a new child DPRTC under the root DPRC.\n"
		"   destroy - destroys a child DPRTC under the root DPRC.\n"
		"\n"
		"For command-specific help, use the --help option of each command.\n"
		"\n";

	printf(help_msg);
	return 0;
}

static int print_dprtc_attr(uint32_t dprtc_id,
			struct dprc_obj_desc *target_obj_desc)
{
	uint16_t dprtc_handle;
	int error;
	struct dprtc_attr dprtc_attr;
	bool dprtc_opened = false;

	error = dprtc_open(&restool.mc_io, 0, dprtc_id, &dprtc_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	dprtc_opened = true;
	if (0 == dprtc_handle) {
		DEBUG_PRINTF(
			"dprtc_open() returned invalid handle (auth 0) for dprtc.%u\n",
			dprtc_id);
		error = -ENOENT;
		goto out;
	}

	memset(&dprtc_attr, 0, sizeof(dprtc_attr));
	error = dprtc_get_attributes(&restool.mc_io, 0, dprtc_handle,
				     &dprtc_attr);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	assert(dprtc_id == (uint32_t)dprtc_attr.id);

	printf("dprtc version: %u.%u\n", dprtc_attr.version.major,
	       dprtc_attr.version.minor);
	printf("dprtc id: %d\n", dprtc_attr.id);
	printf("plugged state: %splugged\n",
		(target_obj_desc->state & DPRC_OBJ_STATE_PLUGGED) ? "" : "un");
	print_obj_label(target_obj_desc);

	error = 0;
out:
	if (dprtc_opened) {
		int error2;

		error2 = dprtc_close(&restool.mc_io, 0, dprtc_handle);
		if (error2 < 0) {
			mc_status = flib_error_to_mc_status(error2);
			ERROR_PRINTF("MC error: %s (status %#x)\n",
				     mc_status_to_string(mc_status), mc_status);
			if (error == 0)
				error = error2;
		}
	}

	return error;
}

static int print_dprtc_info(uint32_t dprtc_id)
{
	int error;
	struct dprc_obj_desc target_obj_desc;
	uint32_t target_parent_dprc_id;
	bool found = false;

	memset(&target_obj_desc, 0, sizeof(struct dprc_obj_desc));
	error = find_target_obj_desc(restool.root_dprc_id,
				restool.root_dprc_handle, 0, dprtc_id,
				"dprtc", &target_obj_desc,
				&target_parent_dprc_id, &found);
	if (error < 0)
		goto out;

	if (strcmp(target_obj_desc.type, "dprtc")) {
		printf("dprtc.%d does not exist\n", dprtc_id);
		return -EINVAL;
	}

	error = print_dprtc_attr(dprtc_id, &target_obj_desc);
	if (error < 0)
		goto out;

	if (restool.cmd_option_mask & ONE_BIT_MASK(INFO_OPT_VERBOSE)) {
		restool.cmd_option_mask &= ~ONE_BIT_MASK(INFO_OPT_VERBOSE);
		error = print_obj_verbose(&target_obj_desc, &dprtc_ops);
	}

out:
	return error;
}

static int cmd_dprtc_info(void)
{
	static const char usage_msg[] =
	"\n"
	"Usage: restool dprtc info <dprtc-object> [--verbose]\n"
	"   e.g. restool dprtc info dprtc.5\n"
	"\n"
	"--verbose\n"
	"   Shows extended/verbose information about the object\n"
	"   e.g. restool dprtc info dprtc.5 --verbose\n"
	"\n";

	uint32_t obj_id;
	int error;

	if (restool.cmd_option_mask & ONE_BIT_MASK(INFO_OPT_HELP)) {
		printf(usage_msg);
		restool.cmd_option_mask &= ~ONE_BIT_MASK(INFO_OPT_HELP);
		error = 0;
		goto out;
	}

	if (restool.obj_name == NULL) {
		ERROR_PRINTF("<object> argument missing\n");
		printf(usage_msg);
		error = -EINVAL;
		goto out;
	}

	error = parse_object_name(restool.obj_name, "dprtc", &obj_id);
	if (error < 0)
		goto out;

	error = print_dprtc_info(obj_id);
out:
	return error;
}

static int cmd_dprtc_create(void)
{
	static const char usage_msg[] =
		"\n"
		"Usage: restool dprtc create [OPTIONS]\n"
		"   e.g. create a DPRTC object with all default options:\n"
		"	restool dprtc create\n"
		"\n"
		"OPTIONS:\n"
		"if options are not specified, create DPRTC by default options\n"
		"--options=<place holder>\n"
		"   Default value is 0\n"
		"   e.g. restool dprtc create --options=5\n"
		"\n";

	int error;
	long val;
	char *endptr;
	char *str;
	struct dprtc_cfg dprtc_cfg;
	uint16_t dprtc_handle;
	struct dprtc_attr dprtc_attr;

	if (restool.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_HELP)) {
		printf(usage_msg);
		restool.cmd_option_mask &= ~ONE_BIT_MASK(CREATE_OPT_HELP);
		return 0;
	}

	if (restool.obj_name != NULL) {
		ERROR_PRINTF("Unexpected argument: \'%s\'\n\n",
			     restool.obj_name);
		printf(usage_msg);
		return -EINVAL;
	}

	if (restool.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_OPTIONS)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_OPTIONS);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_OPTIONS];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) || (val < 0)) {
			printf(usage_msg);
			return -EINVAL;
		}

		dprtc_cfg.options = val;
	} else {
		dprtc_cfg.options = 0;
	}

	error = dprtc_create(&restool.mc_io, 0, &dprtc_cfg, &dprtc_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		return error;
	}

	memset(&dprtc_attr, 0, sizeof(struct dprtc_attr));
	error = dprtc_get_attributes(&restool.mc_io, 0, dprtc_handle,
				     &dprtc_attr);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		return error;
	}
	print_new_obj("dprtc", dprtc_attr.id, NULL);

	error = dprtc_close(&restool.mc_io, 0, dprtc_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		return error;
	}
	return 0;
}

static int cmd_dprtc_destroy(void)
{
	static const char usage_msg[] =
		"\n"
		"Usage: restool dprtc destroy <dprtc-object>\n"
		"   e.g. restool dprtc destroy dprtc.9\n"
		"\n";

	int error;
	int error2;
	uint32_t dprtc_id;
	uint16_t dprtc_handle;
	bool dprtc_opened = false;

	if (restool.cmd_option_mask & ONE_BIT_MASK(DESTROY_OPT_HELP)) {
		printf(usage_msg);
		restool.cmd_option_mask &= ~ONE_BIT_MASK(DESTROY_OPT_HELP);
		return 0;
	}

	if (restool.obj_name == NULL) {
		ERROR_PRINTF("<object> argument missing\n");
		printf(usage_msg);
		error = -EINVAL;
		goto out;
	}

	if (in_use(restool.obj_name, "destroyed")) {
		error = -EBUSY;
		goto out;
	}

	error = parse_object_name(restool.obj_name, "dprtc", &dprtc_id);
	if (error < 0)
		goto out;

	if (!find_obj("dprtc", dprtc_id)) {
		error = -EINVAL;
		goto out;
	}

	error = dprtc_open(&restool.mc_io, 0, dprtc_id, &dprtc_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	dprtc_opened = true;
	if (0 == dprtc_handle) {
		DEBUG_PRINTF(
			"dprtc_open() returned invalid handle (auth 0) for dprtc.%u\n",
			dprtc_id);
		error = -ENOENT;
		goto out;
	}

	error = dprtc_destroy(&restool.mc_io, 0, dprtc_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	dprtc_opened = false;
	printf("dprtc.%u is destroyed\n", dprtc_id);

out:
	if (dprtc_opened) {
		error2 = dprtc_close(&restool.mc_io, 0, dprtc_handle);
		if (error2 < 0) {
			mc_status = flib_error_to_mc_status(error2);
			ERROR_PRINTF("MC error: %s (status %#x)\n",
				     mc_status_to_string(mc_status), mc_status);
			if (error == 0)
				error = error2;
		}
	}

	return error;
}

struct object_command dprtc_commands[] = {
	{ .cmd_name = "help",
	  .options = NULL,
	  .cmd_func = cmd_dprtc_help },

	{ .cmd_name = "info",
	  .options = dprtc_info_options,
	  .cmd_func = cmd_dprtc_info },

	{ .cmd_name = "create",
	  .options = dprtc_create_options,
	  .cmd_func = cmd_dprtc_create },

	{ .cmd_name = "destroy",
	  .options = dprtc_destroy_options,
	  .cmd_func = cmd_dprtc_destroy },

	{ .cmd_name = NULL },
};
