/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <thread>
#include <condition_variable>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"
#include "lib/mgmt.h"
#include "src/shared/mgmt.h"

#include <btio/btio.h>
#include "att.h"
#include "gattrib.h"
#include "gatt.h"
#include "gatttool.h"
#include "version.h"

#include "bluepy-dev.h"

#define IO_CAPABILITY_NOINPUTNOOUTPUT 0x03

#define SHORT_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef BLUEPY_DEBUG
#define DBG(fmt, ...)                                                                          \
    do                                                                                         \
    {                                                                                          \
        printf("# [%s:%d]%s() :" fmt "\n", SHORT_FILE, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        fflush(stdout);                                                                        \
    } while (0)
#else
#ifdef BLUEPY_DEBUG_FILE_LOG
static FILE *fp = nullptr;

static void try_open()
{
    if (!fp)
    {
        fp = fopen("bluepy-helper.log", "w");
    }
}
#define DBG(fmt, ...)                                                    \
    do                                                                   \
    {                                                                    \
        try_open();                                                      \
        if (fp)                                                          \
        {                                                                \
            fprintf(fp, "%s() :" fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
            fflush(fp);                                                  \
        }                                                                \
    } while (0)

#else
#define DBG(fmt, ...)
#endif
#endif

static uint16_t mgmt_ind = MGMT_INDEX_NONE;
static struct mgmt *mgmt_master = nullptr;
struct characteristic_data
{
    uint16_t orig_start;
    uint16_t start;
    uint16_t end;
    bt_uuid_t uuid;
};

static enum state {
    STATE_DISCONNECTED = 0,
    STATE_CONNECTING = 1,
    STATE_CONNECTED = 2,
    STATE_SCANNING = 3,
} conn_state;

// delimits fields in response message
#define RESP_DELIM "\x1e"

static void set_state(enum state st)
{
    conn_state = st;
}

static void scan_cb(uint8_t status, uint16_t length, const void *param, void *user_data)
{
    DBG("------>status: %d", status);
    if (status != MGMT_STATUS_SUCCESS)
    {
        DBG("Scan error: %s (0x%02x)", mgmt_errstr(status), status);
    }
}

// Unlike Bluez, we follow BT 4.0 spec which renammed Device Discovery by Scan
void bluepy_scan(bool start)
{
    // mgmt_cp_start_discovery and mgmt_cp_stop_discovery are the same
    // struct mgmt_cp_start_discovery cp = {(1 << BDADDR_LE_PUBLIC)};
    struct mgmt_cp_start_discovery cp = {(1 << BDADDR_LE_PUBLIC) | (1 << BDADDR_LE_RANDOM)};
    uint16_t opcode = start ? MGMT_OP_START_DISCOVERY : MGMT_OP_STOP_DISCOVERY;

    if (!mgmt_master)
    {
        // resp_error(err_NO_MGMT);
        DBG("err_NO_MGMT");

        return;
    }

    DBG("Scan %s", start ? "start" : "stop");

    if (mgmt_send(mgmt_master, opcode, mgmt_ind, sizeof(cp),
                  &cp, scan_cb, nullptr, nullptr) == 0)
    {
        DBG("mgmt_send(MGMT_OP_%s_DISCOVERY) failed", start ? "START" : "STOP");
        // resp_mgmt(err_SEND_FAIL);
        DBG("err_SEND_FAIL");
        return;
    }
}

static void read_version_complete(uint8_t status, uint16_t length,
                                  const void *param, void *user_data)
{
    const struct mgmt_rp_read_version *rp = static_cast<const mgmt_rp_read_version *>(param);

    if (status != MGMT_STATUS_SUCCESS)
    {
        DBG("Failed to read version information: %s (0x%02x)",
            mgmt_errstr(status), status);
        return;
    }

    if (length < sizeof(*rp))
    {
        DBG("Wrong size of read version response");
        return;
    }

    DBG("Bluetooth management interface %u.%u initialized",
        rp->version, btohs(rp->revision));
}

static void mgmt_device_connected(uint16_t index, uint16_t length,
                                  const void *param, void *user_data)
{
    const struct mgmt_ev_device_connected *ev = static_cast<const mgmt_ev_device_connected *>(param);
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             ev->addr.bdaddr.b[0], ev->addr.bdaddr.b[1], ev->addr.bdaddr.b[2],
             ev->addr.bdaddr.b[3], ev->addr.bdaddr.b[4], ev->addr.bdaddr.b[5]);
    DBG("New device connected: %s", mac_str);

    // DBG("Scan | %s [RSSI: %d] %s\r\n",
    //     mac_str, -ev->rssi, ev->addr.type == BDADDR_LE_PUBLIC ? "public" : "random");
    // printf("device | %s [RSSI: %d] %s\r\n",
    //        mac_str, -ev->rssi, ev->addr.type == BDADDR_LE_PUBLIC ? "public" : "random");
}

static void mgmt_scanning(uint16_t index, uint16_t length,
                          const void *param, void *user_data)
{
    const struct mgmt_ev_discovering *ev = static_cast<const mgmt_ev_discovering *>(param);
    assert(length == sizeof(*ev));

    DBG("Scanning (0x%x): %s", ev->type, ev->discovering ? "started" : "ended");

    set_state(ev->discovering ? STATE_SCANNING : STATE_DISCONNECTED);
}

static void mgmt_device_found(uint16_t index, uint16_t length,
                              const void *param, void *user_data)
{
    const struct mgmt_ev_device_found *ev = static_cast<const mgmt_ev_device_found *>(param);
    // const uint8_t *val = ev->addr.bdaddr.b;
    assert(length == sizeof(*ev) + ev->eir_len);
    // DBG("Device found: %02X:%02X:%02X:%02X:%02X:%02X type=%X flags=%X", val[5], val[4], val[3], val[2], val[1], val[0], ev->addr.type, ev->flags);

    // Result sometimes sent too early
    if (conn_state != STATE_SCANNING)
        return;
    // confirm_name(&ev->addr, 1);

    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             ev->addr.bdaddr.b[0], ev->addr.bdaddr.b[1], ev->addr.bdaddr.b[2],
             ev->addr.bdaddr.b[3], ev->addr.bdaddr.b[4], ev->addr.bdaddr.b[5]);

    std::cout << "Scan | " << mac_str << " [RSSI: " << -ev->rssi << "] "
              << (ev->addr.type == BDADDR_LE_PUBLIC ? "public" : "random") << std::endl;
}

static void mgmt_debug(const char *str, void *user_data)
{
    DBG("%s%s", static_cast<const char *>(user_data), str);
}

struct mgmt *mgmt_setup(unsigned int idx)
{
    mgmt_master = mgmt_new_default();
    if (!mgmt_master)
    {
        DBG("Could not connect to the BT management interface, try with su rights");
        return nullptr;
    }
    DBG("Setting up mgmt on hci%u", idx);
    mgmt_ind = idx;
    mgmt_set_debug(mgmt_master, mgmt_debug, (void *)"mgmt: ", nullptr);

    if (mgmt_send(mgmt_master, MGMT_OP_READ_VERSION,
                  MGMT_INDEX_NONE, 0, nullptr,
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
    return mgmt_master;
}

int bluepy_init()
{
    mgmt_setup(0);

    printf("Starting loop\n");
    bluepy_scan(true);

    DBG("Exiting loop\n");
    return 0;
}
