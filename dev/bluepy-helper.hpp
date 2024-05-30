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
#include <unistd.h>
#include <thread>
#include <memory>
namespace bluepy
{

    class DefaultDelegate
    {
    public:
        DefaultDelegate();
        ~DefaultDelegate();
    };
    using namespace std;

    class BLEZpp
    {
    private:
    public:
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

        struct ConnectState
        {
            enum State
            {
                Disconnected,
                Connecting,
                Connected,
                Disconnecting,
            } state;

            ConnectState(State s)
                : state(s)
            {
            }
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

    private:
        static int connected_handler(ConnectState);
        static int disconnect_handler(Disconnect);
        static int pair_handler(PairState);
        static void scan_complete(uint8_t status, uint16_t length, const void *param, void *user_data);

    public:
        BLEZpp();
        ~BLEZpp();

        std::function<void(ConnectState)> cb_connected = connected_handler;
        std::function<void(Disconnect)> cb_disconnected = disconnect_handler;
        std::function<void(PairState)> cb_pair = pair_handler;
        int ble_mgmt_init(std::condition_variable &cv, std::mutex &mtx, bool &init_done);

        // 扫描、停止扫描
        int scan(std::function<void()> &cb_scan_result, int timeout);
        int stop_scan();

        int connect(std::string device_address);
        int connect(std::string device_address, std::function<void()> &cb_connect_result);

        int pair(std::function<void(PairState)> &cb);
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

    class EventLoop
    {
    public:
        EventLoop() : running(false) {}
        ~EventLoop() { stop(); }

        void start();
        void stop();
        void postEvent(const std::function<void()> &event);

    private:
        void run();
        std::atomic<bool> running;
        std::thread loopThread;
        std::mutex eventMutex;
        std::condition_variable eventCondition;
        std::queue<std::function<void()>> eventQueue;
    };
}
#endif // __BLUEPY_HELPER_H__
