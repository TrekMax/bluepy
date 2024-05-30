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
#include "blezpp.hpp"

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
#define DBG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
using namespace std;
namespace bluepy
{

    int BLEMgmt::bluetooth_mgmt_init(int hic_dev)
    {
        // mgmt_setup(hic_dev);
        mgmt_ind = hic_dev;

        mgmt_master = mgmt_new_default();
        if (!mgmt_master)
        {
            DBG("Could not connect to the BT management interface, try with su rights");
            return -1;
        }
        DBG("Setting up mgmt on hci%u", mgmt_ind);
        mgmt_set_debug(mgmt_master, mgmt_debug, (void *)"mgmt: ", nullptr);

        if (mgmt_send(mgmt_master, MGMT_OP_READ_VERSION, MGMT_INDEX_NONE, 0, nullptr,
                      read_version_complete, nullptr, nullptr) == 0)
        {
            DBG("mgmt_send(MGMT_OP_READ_VERSION) failed");
        }

        if (!mgmt_register(mgmt_master, MGMT_EV_DEVICE_CONNECTED, mgmt_ind, mgmt_device_connected, nullptr, nullptr))
        {
            DBG("mgmt_register(MGMT_EV_DEVICE_CONNECTED) failed");
        }

        if (!mgmt_register(mgmt_master, MGMT_EV_DISCOVERING, mgmt_ind, mgmt_scanning, nullptr, nullptr))
        {
            DBG("mgmt_register(MGMT_EV_DISCOVERING) failed");
        }

        if (!mgmt_register(mgmt_master, MGMT_EV_DEVICE_FOUND, mgmt_ind, mgmt_device_found, nullptr, nullptr))
        {
            DBG("mgmt_register(MGMT_EV_DEVICE_FOUND) failed");
        }
        return 0;
    }
    int BLEMgmt::scan(bool enable)
    {

        // mgmt_cp_start_discovery and mgmt_cp_stop_discovery are the same
        // struct mgmt_cp_start_discovery cp = {(1 << BDADDR_LE_PUBLIC)};
        struct mgmt_cp_start_discovery cp = {(1 << BDADDR_LE_PUBLIC) | (1 << BDADDR_LE_RANDOM)};
        uint16_t opcode = enable ? MGMT_OP_START_DISCOVERY : MGMT_OP_STOP_DISCOVERY;

        if (!mgmt_master)
        {
            // resp_error(err_NO_MGMT);
            DBG("err_NO_MGMT");

            return -1;
        }

        DBG("Scan %s", enable ? "start" : "stop");

        if (mgmt_send(mgmt_master, opcode, mgmt_ind, sizeof(cp),
                      &cp, scan_cb, nullptr, nullptr) == 0)
        {
            DBG("mgmt_send(MGMT_OP_%s_DISCOVERY) failed", enable ? "START" : "STOP");
            // resp_mgmt(err_SEND_FAIL);
            DBG("err_SEND_FAIL");
            return -1;
        }
        return 0;
    }
    void BLEMgmt::read_version_complete(uint8_t status, uint16_t length, const void *param, void *user_data)
    {
        // std::cout << "Read version complete, status: " << static_cast<int>(status) << std::endl;
        const mgmt_rp_read_version *rp = static_cast<const mgmt_rp_read_version *>(param);
        if (status != MGMT_STATUS_SUCCESS)
        {
            std::cout << "Failed to read version information: " << mgmt_errstr(status)
                      << " (0x" << std::hex << static_cast<int>(status) << ")" << std::endl;
            return;
        }

        if (length < sizeof(*rp))
        {
            std::cout << "Wrong size of read version response" << std::endl;
            return;
        }

        std::cout << "Bluetooth management interface " << static_cast<int>(rp->version)
                  << "." << btohs(rp->revision) << " initialized" << std::endl;
    }
    void BLEMgmt::mgmt_device_connected(uint16_t index, uint16_t length, const void *param, void *user_data)
    {
        // std::cout << "Device connected, index: " << index << std::endl;
        const struct mgmt_ev_device_connected *ev = static_cast<const mgmt_ev_device_connected *>(param);
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                 ev->addr.bdaddr.b[0], ev->addr.bdaddr.b[1], ev->addr.bdaddr.b[2],
                 ev->addr.bdaddr.b[3], ev->addr.bdaddr.b[4], ev->addr.bdaddr.b[5]);
        DBG("New device connected: %s", mac_str);
    }
    void BLEMgmt::mgmt_scanning(uint16_t index, uint16_t length, const void *param, void *user_data)
    {
        std::cout << "Scanning, index: " << index << std::endl;
    }

    void BLEMgmt::mgmt_device_found(uint16_t index, uint16_t length, const void *param, void *user_data)
    {
        // std::cout << "Device found, index: " << index << std::endl;
        const struct mgmt_ev_device_found *ev = static_cast<const mgmt_ev_device_found *>(param);
        // const uint8_t *val = ev->addr.bdaddr.b;
        assert(length == sizeof(*ev) + ev->eir_len);
        // DBG("Device found: %02X:%02X:%02X:%02X:%02X:%02X type=%X flags=%X", val[5], val[4], val[3], val[2], val[1], val[0], ev->addr.type, ev->flags);

        // Result sometimes sent too early
        // if (conn_state != STATE_SCANNING)
        //     return;
        // confirm_name(&ev->addr, 1);

        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                 ev->addr.bdaddr.b[0], ev->addr.bdaddr.b[1], ev->addr.bdaddr.b[2],
                 ev->addr.bdaddr.b[3], ev->addr.bdaddr.b[4], ev->addr.bdaddr.b[5]);

        std::cout << "Scan | " << mac_str << " [RSSI: " << -ev->rssi << "] "
                  << (ev->addr.type == BDADDR_LE_PUBLIC ? "public" : "random") << std::endl;
    }

    void BLEMgmt::mgmt_debug(const char *str, void *user_data)
    {
        // std::cout << "Debug: " << str << std::endl;
    }
    void BLEMgmt::scan_cb(uint8_t status, uint16_t length, const void *param, void *user_data)
    {
        // DBG("------>status: %d", status);
        std::cout << "------>status: " << status << std::endl;
        // if (status != MGMT_STATUS_SUCCESS)
        // {
        //     DBG("Scan error: %s (0x%02x)", mgmt_errstr(status), status);
        // }
    }
    DefaultDelegate::DefaultDelegate() {

    }

    DefaultDelegate::~DefaultDelegate() {

    }

    BLEZpp::BLEZpp() 
    {
    }

    BLEZpp::~BLEZpp()
    {
    }

    int BLEZpp::ble_mgmt_init(std::condition_variable &cv, std::mutex &mtx, bool &init_done)
    {
        mgmt.bluetooth_mgmt_init(0);
        mgmt.scan(true);
        {
            std::lock_guard<std::mutex> lock(mtx);
            init_done = true;
        }
        cv.notify_one();
        return 0;
    }

    int BLEZpp::connected_handler(ConnectState)
    {
        return 0;
    }

    int BLEZpp::disconnect_handler(Disconnect)
    {
        return 0;
    }

    int BLEZpp::pair_handler(PairState)
    {
        return 0;
    }
    void BLEZpp::scan_complete(uint8_t status, uint16_t length,
        const void *param, void *user_data)
    {
        cout << "scan complete" << endl;
    }

    // 扫描、停止扫描
    int BLEZpp::scan(std::function<void()> &cb_scan_result, int timeout)
    {
        cout << "----->Start Scan" << endl;
        mgmt.scan(true);
        return 0;
    }
    int BLEZpp::stop_scan()
    {
        mgmt.scan(false);
        cout << "Scan end" << endl;
        return 0;
    }

    int BLEZpp::connect(std::string device_address)
    {
        cout << "connect to " << device_address << endl;
        return 0;
    }

    int BLEZpp::connect(std::string device_address, std::function<void()> &cb)
    {
        cout << "connect to " << device_address << endl;
        return 0;
    }

    int BLEZpp::pair(std::function<void(PairState)> &cb)
    {
        return 0;
    }
    int BLEZpp::disconnect(std::function<void(Disconnect)> &disconnect_handler)
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
    int BLEZpp::discover_service()
    {
        return 0;
    }
    int BLEZpp::discover_characteristic(uint16_t service_handle)
    {
        return 0;
    }
    int BLEZpp::discover_characteristic(const char *service_uuid)
    {
        return 0;
    }

    // 读写特征值
    int BLEZpp::write_char(uint16_t handle, const char *data, int len)
    {
        return 0;
    }
    int BLEZpp::read_char(uint16_t handle, char *data, int len)
    {
        return 0;
    }

    int BLEZpp::read_char_by_uuid(const char *uuid, char *data, int len)
    {
        return 0;
    }
    int BLEZpp::write_char_by_uuid(const char *uuid, const char *data, int len)
    {
        return 0;
    }

    // 启用特征值通知
    int BLEZpp::enable_notify_by_handler(uint16_t handle, bool enable,
                                               std::function<void(const char *data, int len)> &cb)
    {
        return 0;
    }

    int BLEZpp::enable_notify_by_uuid(const char *uuid, bool enable,
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
