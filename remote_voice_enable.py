from bluepy.btle import Peripheral, DefaultDelegate, UUID
import time

# 已知的蓝牙设备地址
device_address = "0C:F5:33:16:64:7B"  # 替换为实际设备地址
# device_address = "0c:f5:33:41:1a:da"
service_uuid = UUID("00001812-0000-1000-8000-00805f9b34fb")

# 代理类，用于处理通知


class NotificationDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        print(f"Received notification: {cHandle} - {data}")


# 连接到设备
print(f"Connecting to device {device_address}...")
device = Peripheral(device_address, "public")
# time.sleep(1)
# device.pair()

try:
    # 获取特定服务
    hid_service = device.getServiceByUUID(service_uuid)
    hid_characteristics_list = hid_service.getCharacteristics()

    # 检查服务中有足够的特征
    if len(hid_characteristics_list) >= 12:
        try:
            device.pair()
            print("Paired successfully")
        except Exception as e:
            device.unpair()
            # print(f"Pairing failed: {e}")

        uuid_voice_enable = hid_characteristics_list[11]
        print(f"Writing to characteristic: {uuid_voice_enable.uuid}")

        # # 写入值 '01'
        uuid_voice_enable.write(b'\x01')
        print("Write successful")

        uuid_voice_notify = hid_characteristics_list[10]
        print(
            f"Enabling notifications for characteristic: {uuid_voice_notify.uuid}")

        # 设置代理并开始监听通知
        delegate = NotificationDelegate()
        with device.withDelegate(delegate):
            # 启用通知
            uuid_voice_notify.write(b'\x01\x00')  # 启用通知的典型值
            count = 0
            # 持续监听
            while True:
                if count >= 5:
                    # uuid_voice_notify.write(b'\x00\x00')  # 启用通知的典型值
                    print("---->Disable RemoteVoice Voice Data")
                    uuid_voice_enable.write(b'\x00')
                    # device.unpair()
                    # break
                print("sleep")
                time.sleep(1)
                if device.waitForNotifications(3.0):
                    print("Waiting for notifications...")
                    # continue
                count += 1

    else:
        print(f"Service {service_uuid} does not have 11 characteristics")
finally:
    # 确保断开连接
    device.unpair()
    device.disconnect()
    print("Disconnected")
    print("---End---")
