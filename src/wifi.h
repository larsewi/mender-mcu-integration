/*
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#if defined(CONFIG_WIFI)
void wifi_connect(void);
#else
#define wifi_connect()
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WIFI_H__ */

