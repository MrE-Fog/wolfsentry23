/*
 * wolfsentry_json.h
 *
 * Copyright (C) 2021-2023 wolfSSL Inc.
 *
 * This file is part of wolfSentry.
 *
 * wolfSentry is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSentry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifndef WOLFSENTRY_JSON_H
#define WOLFSENTRY_JSON_H

#include "wolfsentry.h"

#ifdef WOLFSENTRY_NO_STDIO
#error wolfsentry_json requires stdio
#endif

#ifndef WOLFSENTRY
#define WOLFSENTRY
#endif
#include "centijson_sax.h"

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_centijson_errcode_translate(wolfsentry_errcode_t centijson_errcode);

#ifndef WOLFSENTRY_MAX_JSON_NESTING
#define WOLFSENTRY_MAX_JSON_NESTING 16
#endif

typedef uint32_t wolfsentry_config_load_flags_t;
enum {
    WOLFSENTRY_CONFIG_LOAD_FLAG_NONE             = 0U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_NO_FLUSH         = 1U << 0U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_DRY_RUN          = 1U << 1U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_LOAD_THEN_COMMIT = 1U << 2U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_NO_ROUTES_OR_EVENTS = 1U << 3U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_JSON_DOM_DUPKEY_ABORT = 1U << 4U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_JSON_DOM_DUPKEY_USEFIRST = 1U << 5U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_JSON_DOM_DUPKEY_USELAST = 1U << 6U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_JSON_DOM_MAINTAINDICTORDER = 1U << 7U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_FLUSH_ONLY_ROUTES = 1U << 8U,
    WOLFSENTRY_CONFIG_LOAD_FLAG_FINI             = 1U << 30U
};

struct wolfsentry_json_process_state;

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_config_json_init(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    wolfsentry_config_load_flags_t load_flags,
    struct wolfsentry_json_process_state **jps);

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_config_json_init_ex(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    wolfsentry_config_load_flags_t load_flags,
    const JSON_CONFIG *json_config,
    struct wolfsentry_json_process_state **jps);

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_config_json_feed(
    struct wolfsentry_json_process_state *jps,
    const unsigned char *json_in,
    size_t json_in_len,
    char *err_buf,
    size_t err_buf_size);

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_config_centijson_errcode(struct wolfsentry_json_process_state *jps, int *json_errcode, const char **json_errmsg);

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_config_json_fini(
    struct wolfsentry_json_process_state **jps,
    char *err_buf,
    size_t err_buf_size);

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_config_json_oneshot(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    const unsigned char *json_in,
    size_t json_in_len,
    wolfsentry_config_load_flags_t load_flags,
    char *err_buf,
    size_t err_buf_size);

WOLFSENTRY_API wolfsentry_errcode_t wolfsentry_config_json_oneshot_ex(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    const unsigned char *json_in,
    size_t json_in_len,
    wolfsentry_config_load_flags_t load_flags,
    const JSON_CONFIG *json_config,
    char *err_buf,
    size_t err_buf_size);

#endif /* WOLFSENTRY_JSON_H */
