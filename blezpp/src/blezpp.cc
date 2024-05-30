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

#ifndef _USE_BLUEZ_HEAD
#define _USE_BLUEZ_HEAD
    #include <string.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <stdio.h>
    #include <assert.h>

    #include <btio/btio.h>
    #include <glib.h>

    #include "lib/bluetooth.h"
    #include "lib/sdp.h"
    #include "lib/uuid.h"
    #include "lib/mgmt.h"
    #include "src/shared/mgmt.h"
    #include "att.h"
    #include "gattrib.h"
    #include "gatt.h"
#endif
#include "gatttool.h"
#include <glib.h>
using namespace std;
#define IO_CAPABILITY_NOINPUTNOOUTPUT 0x03
typedef struct mgmt_rp_read_version mgmt_rp_read_version;
namespace blezpp
{
    int BLEMgmt::bluetooth_mgmt_init(int hic_dev)
    {
        cout << "HIC Dev: " << hic_dev << endl;
        mgmt_ind = hic_dev;

        mgmt_master = mgmt_new_default();
        if (!mgmt_master)
        {
            cout << "Could not connect to the BT management interface, try with su rights" << mgmt_ind << endl;
            return -1;
        }
        cout << "Setting up mgmt on hci" << mgmt_ind << endl;
        mgmt_set_debug(mgmt_master, mgmt_debug, (void *)"mgmt: ", nullptr);

        if (mgmt_send(mgmt_master, MGMT_OP_READ_VERSION, MGMT_INDEX_NONE, 0, nullptr,
                      read_version_complete, nullptr, nullptr) == 0)
        {
            cout << "mgmt_send(MGMT_OP_READ_VERSION) failed" << endl;
        }

        if (!mgmt_register(mgmt_master, MGMT_EV_DEVICE_CONNECTED, mgmt_ind, mgmt_device_connected, nullptr, nullptr))
        {
            cout << "mgmt_register(MGMT_EV_DEVICE_CONNECTED) failed" << endl;
        }

        if (!mgmt_register(mgmt_master, MGMT_EV_DISCOVERING, mgmt_ind, mgmt_scanning, nullptr, nullptr))
        {
            cout << "mgmt_register(MGMT_EV_DISCOVERING) failed" << endl;
        }

        if (!mgmt_register(mgmt_master, MGMT_EV_DEVICE_FOUND, mgmt_ind, mgmt_device_found, nullptr, nullptr))
        {
            cout << "mgmt_register(MGMT_EV_DEVICE_FOUND) failed" << endl;
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
            cout << "err_NO_MGMT" << endl;
            return -1;
        }

        cout << "Scan " << (enable ? "start" : "stop") << endl;

        if (mgmt_send(mgmt_master, opcode, mgmt_ind, sizeof(cp),
                      &cp, scan_cb, nullptr, nullptr) == 0)
        {
            cout << "mgmt_send(MGMT_OP_" << (enable ? "START" : "STOP") << "_DISCOVERY) failed" << endl;
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
        // DBG("New device connected: %s", mac_str);
        std::cout << "New device connected: " << mac_str << std::endl;
    }

    void BLEMgmt::mgmt_scanning(uint16_t index, uint16_t length, const void *param, void *user_data)
    {
        std::cout << "Scanning, index: " << index << std::endl;
    }

    void BLEMgmt::mgmt_device_found(uint16_t index, uint16_t length, const void *param, void *user_data)
    {
        const struct mgmt_ev_device_found *ev = static_cast<const mgmt_ev_device_found *>(param);
        assert(length == sizeof(*ev) + ev->eir_len);
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
        std::cout << "scan_cb: status is: 0x" << std::hex << static_cast<int>(status) << std::dec << std::endl;
        if (status != MGMT_STATUS_SUCCESS)
        {
            std::cout << "Scan error: " << mgmt_errstr(status) << std::endl;
            return;
        }
    }
    int BLEMgmt::set_connect_state(int state)
    {
        connect_state = state;
        return 0;
    }

    void BLEMgmt::events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
    {
        uint8_t *opdu;
        uint8_t evt;
        uint16_t handle, olen;
        size_t plen;

        evt = pdu[0];

        if (evt != ATT_OP_HANDLE_NOTIFY && evt != ATT_OP_HANDLE_IND)
        {
            printf("#Invalid opcode %02X in event handler??\n", evt);
            return;
        }

        assert(len >= 3);
        handle = bt_get_le16(&pdu[1]);

        // resp_begin(evt == ATT_OP_HANDLE_NOTIFY ? rsp_NOTIFY : rsp_IND);
        // send_uint(tag_HANDLE, handle);
        // send_data(pdu + 3, len - 3);
        // resp_end();

        if (evt == ATT_OP_HANDLE_NOTIFY)
            return;

        opdu = g_attrib_get_buffer(attrib, &plen);
        olen = enc_confirmation(opdu, plen);

        if (olen > 0)
            g_attrib_send(attrib, 0, opdu, olen, NULL, NULL, NULL);
    }

    void BLEMgmt::gatts_find_info_req(const uint8_t *pdu, uint16_t len, gpointer user_data)
    {
        uint8_t *opdu;
        uint8_t opcode;
        uint16_t starting_handle, olen;
        size_t plen;

        assert(len == 5);
        opcode = pdu[0];
        starting_handle = bt_get_le16(&pdu[1]);
        /* ending_handle = bt_get_le16(&pdu[3]); */

        opdu = g_attrib_get_buffer(attrib, &plen);
        olen = enc_error_resp(opcode, starting_handle, ATT_ECODE_REQ_NOT_SUPP, opdu, plen);
        if (olen > 0)
            g_attrib_send(attrib, 0, opdu, olen, NULL, NULL, NULL);
    }

    void BLEMgmt::gatts_find_by_type_req(const uint8_t *pdu, uint16_t len, gpointer user_data)
    {
        uint8_t *opdu;
        uint8_t opcode;
        uint16_t starting_handle, olen;
        size_t plen;

        assert(len >= 7);
        opcode = pdu[0];
        starting_handle = bt_get_le16(&pdu[1]);
        /* ending_handle = bt_get_le16(&pdu[3]); */
        /* att_type = bt_get_le16(&pdu[5]); */

        opdu = g_attrib_get_buffer(attrib, &plen);
        olen = enc_error_resp(opcode, starting_handle, ATT_ECODE_REQ_NOT_SUPP, opdu, plen);
        if (olen > 0)
            g_attrib_send(attrib, 0, opdu, olen, NULL, NULL, NULL);
    }

    void BLEMgmt::gatts_read_by_type_req(const uint8_t *pdu, uint16_t len, gpointer user_data)
    {
        uint8_t *opdu;
        uint8_t opcode;
        uint16_t starting_handle, olen;
        size_t plen;

        assert(len == 7 || len == 21);
        opcode = pdu[0];
        starting_handle = bt_get_le16(&pdu[1]);
        /* ending_handle = bt_get_le16(&pdu[3]); */
        if (len == 7)
        {
            /* att_type = bt_get_le16(&pdu[5]); */
        }

        opdu = g_attrib_get_buffer(attrib, &plen);
        olen = enc_error_resp(opcode, starting_handle, ATT_ECODE_REQ_NOT_SUPP, opdu, plen);
        if (olen > 0)
            g_attrib_send(attrib, 0, opdu, olen, NULL, NULL, NULL);
    }
    void BLEMgmt::connect_cb(GIOChannel *io, GError *err, gpointer user_data)
    {
        cout << "connect_cb" << endl;

        uint16_t mtu;
        uint16_t cid;
        GError *gerr = NULL;
        // DBG("io = %p, err = %p", io, err);
        cout << "io = " << io << ", err = " << err << endl;
        if (err)
        {
            // set_state(STATE_DISCONNECTED);
            set_connect_state(Disconnected);
            // resp_str_error(err_CONN_FAIL, err->message);
            printf("# Connect error: %s\n", err->message);
            return;
        }
#if 1

        bt_io_get(io, &gerr, BT_IO_OPT_IMTU, &mtu,
                  BT_IO_OPT_CID, &cid, BT_IO_OPT_INVALID);

        if (gerr)
        {
            printf("# Can't detect MTU, using default");
            g_error_free(gerr);
            mtu = ATT_DEFAULT_LE_MTU;
        }
        else if (cid == ATT_CID)
            mtu = ATT_DEFAULT_LE_MTU;

        attrib = g_attrib_new(iochannel, mtu, false);

        g_attrib_register(attrib, ATT_OP_HANDLE_NOTIFY, GATTRIB_ALL_HANDLES,
                          events_handler, attrib, NULL);
        g_attrib_register(attrib, ATT_OP_HANDLE_IND, GATTRIB_ALL_HANDLES,
                          events_handler, attrib, NULL);
        g_attrib_register(attrib, ATT_OP_FIND_INFO_REQ, GATTRIB_ALL_HANDLES,
                          gatts_find_info_req, attrib, NULL);
        g_attrib_register(attrib, ATT_OP_FIND_BY_TYPE_REQ, GATTRIB_ALL_HANDLES,
                          gatts_find_by_type_req, attrib, NULL);
        g_attrib_register(attrib, ATT_OP_READ_BY_TYPE_REQ, GATTRIB_ALL_HANDLES,
                          gatts_read_by_type_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_READ_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_read_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_READ_BLOB_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_read_blob_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_READ_MULTI_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_read_multi_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_READ_BY_GROUP_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_read_by_group_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_WRITE_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_write_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_WRITE_CMD, GATTRIB_ALL_HANDLES,
        //                   gatts_write_cmd, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_SIGNED_WRITE_CMD, GATTRIB_ALL_HANDLES,
        //                   gatts_signed_write_cmd, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_PREP_WRITE_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_prep_write_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_EXEC_WRITE_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_exec_write_req, attrib, NULL);
        // g_attrib_register(attrib, ATT_OP_MTU_REQ, GATTRIB_ALL_HANDLES,
        //                   gatts_mtu_req, attrib, NULL);

        // connect_state = Connected;
        set_connect_state(Connected);
#endif
    }
    void BLEMgmt::disconnect_cb(uint8_t status, uint16_t length, const void *param, void *user_data)
    {
        cout << "disconnect_cb" << endl;
    }
    void BLEMgmt::pair_device_complete(uint8_t status, uint16_t length,
                                       const void *param, void *user_data)
    {
        if (status != MGMT_STATUS_SUCCESS)
        {
            // DBG("status returned error : %s (0x%02x)",
            //     mgmt_errstr(status), status);
            // resp_mgmt_err(status);
            cout << "status returned error : " << mgmt_errstr(status) << " (0x" << status << ")" << endl;
            return;
        }
        cout << "pair_device_complete" << endl;
        // resp_mgmt(err_SUCCESS);
    }
    void BLEMgmt::pair_cb(uint8_t status, uint16_t length, const void *param, void *user_data)
    {
        cout << "pair_cb" << endl;

        struct mgmt_cp_pair_device cp;
        bdaddr_t bdaddr;
        uint8_t io_cap = IO_CAPABILITY_NOINPUTNOOUTPUT;
        uint8_t addr_type = BDADDR_LE_RANDOM;

        if (!mgmt_master)
        {
            cout << "mgmt_master NO_MGMT" << endl;
            return;
        }

        if (connect_state != ConnectState::Connected)
        {
            // resp_mgmt(err_BAD_STATE);
            cout << "resp_mgmt(err_BAD_STATE)" << endl;
            return;
        }

        if (str2ba(opt_dst, &bdaddr))
        {
            // resp_mgmt(err_NOT_FOUND);
            cout << "resp_mgmt(err_NOT_FOUND)" << endl;
            return;
        }

        if (!memcmp(opt_dst_type, "public", 6))
        {
            addr_type = BDADDR_LE_PUBLIC;
        }

        memset(&cp, 0, sizeof(cp));
        bacpy(&cp.addr.bdaddr, &bdaddr);
        cp.addr.type = addr_type;
        cp.io_cap = io_cap;

        if (mgmt_send(mgmt_master, MGMT_OP_PAIR_DEVICE,
                      mgmt_ind, sizeof(cp), &cp,
                      pair_device_complete, NULL,
                      NULL) == 0)
        {
            // DBG("mgmt_send(MGMT_OP_PAIR_DEVICE) failed for %s for hci%u", opt_dst, mgmt_ind);
            cout << "mgmt_send(MGMT_OP_PAIR_DEVICE) failed for " << opt_dst << " for hci" << mgmt_ind << endl;
            return;
        }
    }
    void BLEMgmt::unpair_cb(uint8_t status, uint16_t length, const void *param, void *user_data)
    {
        std::cout << "unpair_cb" << std::endl;
    }

    int BLEMgmt::channel_watcher(GIOChannel *chan, GIOCondition cond,
                                 gpointer user_data)
    {
        std::cout << "channel_watcher" << std::endl;
        // DBG("chan = %p", chan);

        // in case of quick disconnection/reconnection, do not mix them
        // if (chan == this->iochannel)
            // disconnect_io();

        return FALSE;
    }

    int BLEMgmt::connect(const char *addr)
    {
        cout << "connect to " << addr << endl;
        GError *gerr = NULL;
        // connect_state = Connecting;
        // set_connect_state(Connecting);
        g_free(opt_dst);
        opt_dst = g_strdup(addr);
        opt_dst_type = g_strdup("public");
        g_free(opt_src);
        opt_src = NULL;
        opt_sec_level = g_strdup("low");

        std::cout << "--->1" << std::endl;
        iochannel = gatt_connect(opt_src, opt_dst, opt_dst_type, opt_sec_level, 
            opt_psm, opt_mtu, connect_cb, &gerr);
        std::cout << "--->2" << std::endl;

        if (iochannel == NULL)
        {
            std::cout << "--->3" << std::endl;
            g_error_free(gerr);
        }
        else {
            g_io_add_watch(iochannel, static_cast<GIOCondition>(G_IO_HUP | G_IO_NVAL), channel_watcher, NULL);
        }
        std::cout << "gatt_connect returned" << std::endl;
        return 0;
    }
    int BLEMgmt::pair()
    {
        struct mgmt_cp_pair_device cp;
        bdaddr_t bdaddr;
        uint8_t io_cap = IO_CAPABILITY_NOINPUTNOOUTPUT;
        uint8_t addr_type = BDADDR_LE_PUBLIC;

        if (!mgmt_master)
        {
            // resp_error(err_NO_MGMT);
            cout << "resp_error(err_NO_MGMT)" << endl;
            return -1;
        }

        if (connect_state != ConnectState::Connected)
        {
            // resp_mgmt(err_BAD_STATE);
            cout << "resp_mgmt(err_BAD_STATE)" << endl;
            return -1;
        }

        if (str2ba(opt_dst, &bdaddr))
        {
            // resp_mgmt(err_NOT_FOUND);
            cout << "resp_mgmt(err_NOT_FOUND)" << endl;
            return -1;
        }

        if (!memcmp(opt_dst_type, "public", 6))
        {
            addr_type = BDADDR_LE_PUBLIC;
        }

        memset(&cp, 0, sizeof(cp));
        bacpy(&cp.addr.bdaddr, &bdaddr);
        cp.addr.type = addr_type;
        cp.io_cap = io_cap;

        if (mgmt_send(mgmt_master, MGMT_OP_PAIR_DEVICE,
                      mgmt_ind, sizeof(cp), &cp,
                      pair_device_complete, NULL,
                      NULL) == 0)
        {
            // DBG("mgmt_send(MGMT_OP_PAIR_DEVICE) failed for %s for hci%u", opt_dst, mgmt_ind);
            // resp_mgmt(err_SEND_FAIL);
            cout << "mgmt_send(MGMT_OP_PAIR_DEVICE) failed for " << opt_dst << " for hci" << mgmt_ind << endl;
            return -1;
        }
        return 0;
    }
    int BLEMgmt::unpair()
    {
        return 0;
    }
    int BLEMgmt::disconnect()
    {
        return 0;
    }

    BLEZpp::BLEZpp()
    {
        mainLoop = g_main_loop_new(nullptr, FALSE);
    }

    BLEZpp::~BLEZpp()
    {
        if (mainLoopThread.joinable())
        {
            g_main_loop_quit(mainLoop);
            mainLoopThread.join();
        }
        g_main_loop_unref(mainLoop);
    }

    int BLEZpp::init(int hic_dev)
    {
        cout << "init" << endl;
        mgmt.bluetooth_mgmt_init(0);
        // mgmt.bluetooth_mgmt_init(hic_dev);
        cout << "RunMainLoop" << endl;
        mainLoopThread = std::thread(&BLEZpp::runMainLoop, this);
        cout << "Init done" << endl;
        return 0;
    }

    void BLEZpp::runMainLoop()
    {
        g_main_loop_run(mainLoop);
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

    int BLEZpp::scan_complete(std::string device_address, std::string device_name)
    {
        cout << "scan complete" << endl;
        return 0;
    }

    // 扫描、停止扫描
    int BLEZpp::scan(bool enable)
    {
        return mgmt.scan(enable);
    }

    int BLEZpp::connect(std::string device_address)
    {
        mgmt.connect(device_address.c_str());
        return 0;
    }

    int BLEZpp::connect(std::string device_address, std::function<void()> &cb)
    {
        cout << "connect to " << device_address << endl;
        return 0;
    }
    // int BLEZpp::pair(std::function<void(PairState)> &cb)
    int BLEZpp::pair()
    {
        cout << "---->pair" << endl;
        mgmt.pair();
        return 0;
    }
    int BLEZpp::disconnect(std::function<void(Disconnect)> &disconnect_handler)
    {
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

}
