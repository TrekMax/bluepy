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

#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <thread>
#include <condition_variable>

#include <glib.h>
#include "blezpp.hpp"

using namespace blezpp;
using namespace std;

int main(int argc, char const *argv[])
{
    std::cout << "======================" << std::endl;
    std::cout << __DATE__ <<" " << __TIME__ << std::endl;
    std::cout << "======================" << std::endl;

    BLEZpp blehelper;
    blehelper.init(0);

    std::function<void()> cb_scan_result = []()
    {
        cout<< "Scan callback" << endl;
    };
    std::cout << "Starting loop" << std::endl;

    blehelper.scan(cb_scan_result);

    cout << "Scanning for 10 seconds" << endl;
    int i = 0;
    while (true)
    {
        // 处理业务：扫描、连接、配对、读写
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (i++ > 10)
        {
            cout << "Stop Scan" << endl;
            blehelper.stop_scan();
        }
    }

    return 0;
}
