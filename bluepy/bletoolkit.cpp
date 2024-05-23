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

#include <bluepy-helper.h>

namespace bluepy{

    class DefaultDelegate
    {
    public:
        DefaultDelegate() {}
        ~DefaultDelegate() {}
    };

    using namespace std;
    class BLEToolkit
    {

    public:
        
        BLEToolkit() {}
        ~BLEToolkit(){}
        static int connect_handler(int status)
        {
            return 0;
        }

        void connect(std::string device_address)
        {
            std::cout << "connect to " << device_address << std::endl;
            bluepy_connect(device_address.c_str(), "public", connect_handler);
            // bluepy_connect(device_address.c_str(), "public");
        }
        void disconnect() {
            std::cout << "disconnect" << std::endl;
            // bluepy_disconnect();
        }

        // std::string device_service_uuid, 
        // std::string device_characteristic_uuid 
        // void read_characteristic();
        // void write_characteristic();

    private:
        // std::string device_name;
        // std::string device_address;
        // std::string device_service_uuid;
        // std::string device_characteristic_uuid;
    };
}

using namespace bluepy;

int main(int argc, char const *argv[])
{
    std::cout << "======================" << std::endl;
    std::cout << __DATE__ <<" " << __TIME__ << std::endl;
    std::cout << "======================" << std::endl;

    BLEToolkit ble_toolkit;
    ble_toolkit.connect("0C:F5:33:EF:97:89");
    // ble_toolkit.read_characteristic();
    // ble_toolkit.write_characteristic();
    // ble_toolkit.disconnect();
    return 0;
}

