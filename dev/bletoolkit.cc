/**
 * @file bletoolkit.cpp
 * @author TsMax (QinYUN575@Foxmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-24
 * 
 * @copyright Copyright (c) 2021 - 2024 shenzhen listenai co., ltd.
 * 
 * SPDX-License-Identifier: Apache-2.0
 */
#include <iostream>
#include <string>
#include <thread>
#include "bluepy-helper.hpp"

#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <thread>
#include <condition_variable>

#include <glib.h>

using namespace bluepy;
using namespace std;

void exampleEvent()
{
    cout << "Runnig" << endl;
}

int main(int argc, char const *argv[])
{
    std::cout << "======================" << std::endl;
    std::cout << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "======================" << std::endl;

    BluepyHelper blehelper;
    EventLoop eventLoop;

    // bool enable = true;
    // std::function<void()> connect_handler = [&enable](ConnectState)
    // {
    //     cout<< "Connected callback" << endl;
    // };

    // blehelper.cb_disconnected = [](BluepyHelper::Disconnect d)
    // {
    //     cerr << "Disconnect for reason " << BluepyHelper::get_disconnect_string(d) << endl;
    //     exit(1);
    // };
    // blehelper.connect("0C:F5:33:41:1A:DA");
    // blehelper.connect("0C:F5:33:41:1A:DA", connect_handler);

    std::function<void()> cb_scan_result = []()
    {
        cout << "Scan callback" << endl;
    };
    eventLoop.start();
    std::cout << "Starting loop" << std::endl;
    std::condition_variable cv;
    std::mutex mtx; // 互斥锁
    bool init_done = false;
    // 使用 std::thread 在独立线程中运行 ble_mgmt_init
    std::thread ble_mgmt_thread([&blehelper, &cv, &mtx, &init_done]()
                                {
        cout << "ble_mgmt_init in thread" << endl;
        blehelper.ble_mgmt_init(cv, mtx, init_done);
        static GMainLoop *event_loop;
        event_loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(event_loop); });
    // 等待 ble_mgmt_init 完成
    cout << "等待 ble_mgmt_init 完成" << endl;
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&init_done]()
                { return init_done; });
    }
    // eventLoop.postEvent([&blehelper]()
    //                     { blehelper.ble_mgmt_init(); });

    blehelper.scan(cb_scan_result, 1000);
    cout << "Waiting for scan" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(30));
    eventLoop.stop();

    // 处理扫描、连接、配对、读写
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Exiting loop" << std::endl;
    // 退出时停止 ble_mgmt_thread
    if (ble_mgmt_thread.joinable())
    {
        ble_mgmt_thread.detach(); // 使用 detach 分离线程，避免主线程等待
    }

    return 0;
}
