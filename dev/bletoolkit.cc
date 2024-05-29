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

using namespace bluepy;

int main(int argc, char const *argv[])
{
    std::cout << "======================" << std::endl;
    std::cout << __DATE__ <<" " << __TIME__ << std::endl;
    std::cout << "======================" << std::endl;

    // bool enable = true;
    // std::function<void()> connect_handler = [&enable](ConnectState)
    // {
    //     cout<< "Connected callback" << endl;
    // };
    std::function<void()> cb_scan_result = []()
    {
        cout<< "Connected callback" << endl;
    };

    BluepyHelper helper;

    helper.cb_disconnected = [](BluepyHelper::Disconnect d)
    {
        // cerr << "Disconnect for reason " << BluepyHelper::get_disconnect_string(d) << endl;
        exit(1);
    };
    helper.connect("0C:F5:33:41:1A:DA");
    // helper.connect("0C:F5:33:41:1A:DA", connect_handler);
    helper.scan(cb_scan_result, 1000);
    // helper.stop_scan();
    // while (1)
    // {
    //     std::cout << "Sleep..." << std::endl;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // }

    return 0;
}
