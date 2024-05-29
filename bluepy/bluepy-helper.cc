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
#include "bluepy-helper.hpp"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define CLANG_HEAD
#ifdef CLANG_HEAD
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <glib.h>
#endif

#include <btio/btio.h>
#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"
#include "lib/mgmt.h"
#include "src/shared/mgmt.h"
#include "att.h"
#include "gattrib.h"
#include "gatt.h"
#include "gatttool.h"
#include "version.h"

#include "bluepy-dev.h"

using namespace std;
namespace bluepy
{

    DefaultDelegate::DefaultDelegate() {

    }

    DefaultDelegate::~DefaultDelegate() {

    }

    BluepyHelper::BluepyHelper() 
    {
    }

    BluepyHelper::~BluepyHelper()
    {
    }

    int BluepyHelper::ble_mgmt_init(std::condition_variable &cv, std::mutex &mtx, bool &init_done)
    {
        // 初始化完成后通知主线程
        mgmt_setup(0);
        bluepy_init();
        {
            std::lock_guard<std::mutex> lock(mtx);
            init_done = true;
        }
        cv.notify_one();
        return 0;
    }

    int BluepyHelper::connected_handler(ConnectState)
    {
        return 0;
    }

    int BluepyHelper::disconnect_handler(Disconnect)
    {
        return 0;
    }

    int BluepyHelper::pair_handler(PairState)
    {
        return 0;
    }
    void BluepyHelper::scan_complete(uint8_t status, uint16_t length,
        const void *param, void *user_data)
    {
        cout << "scan complete" << endl;
    }

    // 扫描、停止扫描
    int BluepyHelper::scan(std::function<void()> &cb_scan_result, int timeout)
    {
        cout << "----->Start Scan" << endl;
#if 0
        // mgmt_cp_start_discovery and mgmt_cp_stop_discovery are the same
        struct mgmt_cp_start_discovery cp = {(1 << BDADDR_LE_PUBLIC) | (1 << BDADDR_LE_RANDOM)};
        uint16_t opcode = start ? MGMT_OP_START_DISCOVERY : MGMT_OP_STOP_DISCOVERY;
        if (!mgmt_master)
        {
            // resp_error(err_NO_MGMT);
            cout << "No mgmt" << endl;
            return -1;
        }

        // if (mgmt_send(mgmt_master, opcode, mgmt_ind, sizeof(cp),
        //               &cp, scan_complete, NULL, NULL) == 0)
        if (mgmt_send(mgmt_master, opcode, mgmt_ind, sizeof(cp),
                      &cp, NULL, NULL, NULL) == 0)
        {
            std::cout << "mgmt_send" << std::endl;
            return 0;
        }
#endif
        bluepy_scan(true);
        cout << "Scan end" << endl;
        return 0;
    }
    int BluepyHelper::stop_scan()
    {
        return 0;
    }

    int BluepyHelper::connect(std::string device_address)
    {
        cout << "connect to " << device_address << endl;
        return 0;
    }

    int BluepyHelper::connect(std::string device_address, std::function<void()> &cb)
    {
        cout << "connect to " << device_address << endl;
        return 0;
    }

    int BluepyHelper::pair(std::function<void(PairState)> &cb)
    {
        return 0;
    }
    int BluepyHelper::disconnect(std::function<void(Disconnect)> &disconnect_handler)
    {
#if 0
        if (conn_state == STATE_DISCONNECTED)
            return;

        g_attrib_unref(attrib);
        attrib = NULL;
        opt_mtu = 0;

        g_io_channel_shutdown(iochannel, FALSE, NULL);
        g_io_channel_unref(iochannel);
        iochannel = NULL;

        set_state(STATE_DISCONNECTED);
#endif
        return 0;
    }

    // 发现服务、发现特征
    int BluepyHelper::discover_service()
    {
        return 0;
    }
    int BluepyHelper::discover_characteristic(uint16_t service_handle)
    {
        return 0;
    }
    int BluepyHelper::discover_characteristic(const char *service_uuid)
    {
        return 0;
    }

    // 读写特征值
    int BluepyHelper::write_char(uint16_t handle, const char *data, int len)
    {
        return 0;
    }
    int BluepyHelper::read_char(uint16_t handle, char *data, int len)
    {
        return 0;
    }

    int BluepyHelper::read_char_by_uuid(const char *uuid, char *data, int len)
    {
        return 0;
    }
    int BluepyHelper::write_char_by_uuid(const char *uuid, const char *data, int len)
    {
        return 0;
    }

    // 启用特征值通知
    int BluepyHelper::enable_notify_by_handler(uint16_t handle, bool enable,
                                               std::function<void(const char *data, int len)> &cb)
    {
        return 0;
    }

    int BluepyHelper::enable_notify_by_uuid(const char *uuid, bool enable,
                                            std::function<void(const char *data, int len)> &cb)
    {
        return 0;
    }

    void EventLoop::start()
    {
        running = true;
        loopThread = std::thread([this]()
                                 { this->run(); });
    }

    void EventLoop::stop()
    {
        {
            std::lock_guard<std::mutex> lock(eventMutex);
            running = false;
        }
        eventCondition.notify_all();
        if (loopThread.joinable())
        {
            loopThread.join();
        }
    }

    void EventLoop::postEvent(const std::function<void()> &event)
    {
        {
            std::lock_guard<std::mutex> lock(eventMutex);
            eventQueue.push(event);
        }
        eventCondition.notify_one();
    }

    void EventLoop::run()
    {
        while (running)
        {
            std::function<void()> event;
            {
                std::unique_lock<std::mutex> lock(eventMutex);
                eventCondition.wait(lock, [this]()
                                    { return !eventQueue.empty() || !running; });

                if (!running && eventQueue.empty())
                {
                    return;
                }

                event = eventQueue.front();
                eventQueue.pop();
            }
            event();
        }
    }
}
