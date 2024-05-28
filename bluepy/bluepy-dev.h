/**
 * @file bluepy-dev.h
 * @author TsMax (QinYUN575@Foxmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-29
 * 
 * @copyright Copyright (c) 2021 - 2024 shenzhen listenai co., ltd.
 * 
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BLUEPY_DEV_H__
#define __BLUEPY_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif

int bluepy_init();
struct mgmt *mgmt_setup(unsigned int idx);
void bluepy_scan(bool start);

#ifdef __cplusplus
}
#endif


#endif // __BLUEPY_DEV_H__