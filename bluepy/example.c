/**
 * @file example.c
 * @author TsMax (QinYUN575@Foxmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-23
 * 
 * @copyright Copyright (c) 2021 - 2024 shenzhen listenai co., ltd.
 * 
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bluepy-helper.h"
#include <stdlib.h>


// 1. 扫描获取蓝牙列表
// 2. 连接蓝牙设备
// 3. 读取蓝牙设备信息
// 4. 读取蓝牙设备服务
// 5. 读取蓝牙 语音特征值
// 6. 写入蓝牙 语音特征值
// 7. 监听蓝牙按键操作
// 8. 断开蓝牙设备


int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <mac address>\n", argv[0]);
        return 1;
    }

    char *mac = argv[1];
    int ret = bluepy_connect(mac, "public");

    if (ret < 0) {
        printf("Failed to connect to %s\n", mac);
        exit(-1);
    }

    ret = bluepy_pair(mac, 2000);
    if (ret < 0) {
        printf("Failed to pair with %s\n", mac);
        exit(-1);
    }

    
    ret = bluepy_discoverServices

    return 0;
}