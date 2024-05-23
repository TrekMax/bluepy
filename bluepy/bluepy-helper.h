/**
 * @file bluepy-helper.h
 * @author TianShuang Ke (dske@listenai.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-23
 * 
 * @copyright Copyright (c) 2021 - 2024 shenzhen listenai co., ltd.
 * 
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BLUEPY_HELPER_H__
#define __BLUEPY_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 
 * 
 * @param enable 启用蓝牙扫描
 * @param timeout 扫描超时时间，enable 为 True 时有效
 * @return int
 */
int bluepy_scan(int enable, int timeout);


typedef enum _bluepy_status_t {
    BLUEPY_STATUS_OK = 0,
    BLUEPY_STATUS_ERROR = -1,
    BLUEPY_STATUS_TIMEOUT = -2,
    BLUEPY_STATUS_NOT_FOUND = -3,
} bluepy_status_t;

typedef enum _bluepy_connect_status_t {
    BLUEPY_DISCONNECT = -1,
    BLUEPY_CONNECTED = 0,
    BLUEPY_CONNECTING = 1,
    BLUEPY_DISCONNECTING = 2,
} bluepy_connect_status_t;

typedef int (*connect_cb_t)(int status);

/**
 * @brief 
 * 
 * @param dst 
 * @param dst_type 
 * @return int 
 */
int bluepy_connect(const char *dst, const char *dst_type, const connect_cb_t connect_handler_t);
// int bluepy_connect(const char *dst, const char *dst_type);

#if 0
/**
 * @brief 
 * 
 * @return int 
 */
int bluepy_disconnect();


/**
 * @brief 
 * 
 * @param dst 
 * @param timeout 
 * @return int 
 */
int bluepy_pair(const char *dst, int timeout);


/**
 * @brief 
 * 
 * @param dst 
 * @param timeout 
 * @return int 
 */
int bluepy_unpair(const char *dst, int timeout);


typedef struct _ble_service_t {
    int ser_handler;
    char *ser_uuid;
    char *ser_name;
} ble_service_t;

int bluepy_discoverServices(ble_service_t *service, int service_num, int timeout);


/**
 * @brief 
 * 
 * @param uuid_service 
 * @param timeout 
 * @return int 
 */
int bluepy_get_services(const char *uuid_service, int timeout);

/**
 * @brief 
 * 
 * @param uuid_char 
 * @param timeout
 * @return int 
 */
int bluepy_get_characteristics(const char *uuid_char, int timeout);

/**
 * @brief 
 * 
 * @param uuid_desc 
 * @return int 
 */
int bluepy_get_descriptors(const char *uuid_desc);

int bluepy_get_notify_value(const char *uuid_char);

int bluepy_set_notify_value(const char *uuid_char);

int bluepy_write_value(const char *uuid_char, const char *value, 
                                  int value_len);

int bluepy_read_value(const char *uuid_char, char *value, 
                                 int value_len);
#endif

#ifdef __cplusplus
}
#endif

#endif // __BLUEPY_HELPER_H__

