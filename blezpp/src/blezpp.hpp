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

#include <functional>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <atomic>
#include <condition_variable>
#include <queue>

#include <cstring>
#include <thread>
#include <memory>

#include <glib.h>

#include <sstream>
#include <iomanip>
typedef struct mgmt mgmt;
struct _GAttrib;
typedef struct _GAttrib GAttrib;
namespace blezpp
{

    using namespace std;

    struct Disconnect
    {
        enum Reason
        {
            ConnectionFailed,
            UnexpectedError,
            UnexpectedResponse,
            WriteError,
            ReadError,
            ConnectionClosed,
        } reason;

        static constexpr int NoErrorCode = 1; // Any positive value
        int error_code;

        Disconnect(Reason r, int e)
            : reason(r), error_code(e)
        {
        }
    };

    enum ConnectState
    {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
    };
    struct PairState
    {
        enum State
        {
            Pairing,
            Paired,
            PairFailed,
        } state;

        PairState(State s)
            : state(s)
        {
        }
    };
    template <typename T>
    class Singleton
    {
    public:
        // 禁止拷贝构造和赋值
        Singleton(const Singleton &) = delete;
        Singleton &operator=(const Singleton &) = delete;
        // 获取单例实例的方法
        static T &getInstance()
        {
            static T instance;
            return instance;
        }

    protected:
        // 私有构造函数，确保外部无法实例化
        Singleton() {}
        ~Singleton() {}
    };
    static GIOChannel *iochannel = NULL;
    static GAttrib *attrib = NULL;
    static int connect_state = ConnectState::Disconnected;
    static mgmt *mgmt_master;
    static unsigned int mgmt_ind;
    static gchar *opt_src = NULL;
    static gchar *opt_dst = NULL;
    static gchar *opt_dst_type = NULL;
    static gchar *opt_sec_level = NULL;

    class BLEMgmt : public Singleton<BLEMgmt>
    {
    friend class Singleton<BLEMgmt>; // 允许基类访问私有构造函数
    private:
        BLEMgmt() {}
        ~BLEMgmt() {}

        const int opt_psm = 0;
        int opt_mtu = 0;
        int start;
        int end;

        // static int connect_state;

        static void read_version_complete(uint8_t status, uint16_t length, const void *param, void *user_data);
        static void mgmt_device_connected(uint16_t index, uint16_t length, const void *param, void *user_data);
        static void mgmt_scanning(uint16_t index, uint16_t length, const void *param, void *user_data);
        static void mgmt_device_found(uint16_t index, uint16_t length, const void *param, void *user_data);
        static void mgmt_debug(const char *str, void *user_data);
        static void scan_cb(uint8_t status, uint16_t length, const void *param, void *user_data);
        static void connect_cb(GIOChannel *io, GError *err, gpointer user_data);
        // static void connect_cb(uint8_t status, uint16_t length, const void *param, void *user_data);
        static void disconnect_cb(uint8_t status, uint16_t length, const void *param, void *user_data);
        static void pair_cb(uint8_t status, uint16_t length, const void *param, void *user_data);
        static void unpair_cb(uint8_t status, uint16_t length, const void *param, void *user_data);
        static int channel_watcher(GIOChannel *chan, GIOCondition cond, gpointer user_data);
        static int set_connect_state(int state);

        static void events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data);
        static void gatts_find_info_req(const uint8_t *pdu, uint16_t len, gpointer user_data);
        static void gatts_find_by_type_req(const uint8_t *pdu, uint16_t len, gpointer user_data);
        static void gatts_read_by_type_req(const uint8_t *pdu, uint16_t len, gpointer user_data);

        static void pair_device_complete(uint8_t status, uint16_t length,
                                         const void *param, void *user_data);
    
    public:
        int bluetooth_mgmt_init(int hic_dev);
        int scan(bool enable);
        int connect(const char *addr);
        int pair();
        int unpair();
        int disconnect();
        // int get_version();
        // int get_info(const char *addr);
        // int get_connections();
        // int get_paired_devices();
        // int get_devices();
        // int get_services(const char *addr);
        // int get_characteristics(const char *addr, const char *uuid);
    };

    class BLEZpp
    {
    private:

    public:

    private:
        static int connected_handler(ConnectState);
        static int disconnect_handler(Disconnect);
        static int pair_handler(PairState);
        static int scan_complete(std::string device_address, std::string device_name);

        BLEMgmt &mgmt = BLEMgmt::getInstance();

    private:
        GMainLoop *mainLoop;
        // GIOChannel *iochannel = NULL;
        std::thread mainLoopThread;
        void runMainLoop();

    public:
        BLEZpp();
        ~BLEZpp();

        std::function<void(ConnectState)> cb_connected = connected_handler;
        std::function<void(Disconnect)> cb_disconnected = disconnect_handler;
        std::function<void(PairState)> cb_pair = pair_handler;
        std::function<void(std::string device_address, std::string device_name)> cb_scan_result = scan_complete;
        int init(int hic_dev);

        // 扫描、停止扫描
        int register_scan_callback(std::function<void(std::string device_address, std::string device_name)> &cb_scan_result);
        int scan(bool enable);

        int connect(std::string device_address);
        int connect(std::string device_address, std::function<void()> &cb_connect_result);

        // int pair(std::function<void(PairState)> &cb);
        int pair();
        int disconnect(std::function<void(Disconnect)> &disconnect_handler);

        // 发现服务、发现特征
        int discover_service();
        int discover_characteristic(uint16_t service_handle);
        int discover_characteristic(const char *service_uuid);

        // 读写特征值
        int write_char(uint16_t handle, const char *data, int len);
        int read_char(uint16_t handle, char *data, int len);

        int read_char_by_uuid(const char *uuid, char *data, int len);
        int write_char_by_uuid(const char *uuid, const char *data, int len);

        // 启用特征值通知
        int enable_notify_by_handler(uint16_t handle, bool enable, 
                std::function<void(const char *data, int len)> &cb);

        int enable_notify_by_uuid(const char *uuid, bool enable, 
                std::function<void(const char *data, int len)> &cb);

    };

}
#endif // __BLUEPY_HELPER_H__

