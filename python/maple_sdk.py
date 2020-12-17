import hid

MAPLE_VID = 0x335E
MAPLE_PID = 0x8A01
MAPLE_ENVY_INF = 3
PHONE_REPORT_ID = 0x000B0001


class MapleSdk:

    def __init__(self):
        print("Maple SDK Instantiated")
        with hid.Device(vid=MAPLE_VID, pid=MAPLE_PID) as device:
            self.__print_device(device)
            report = device.read(11, 100)
            print("+++++++++++++++++")
            print(f"report: {report}")

    def __print_device(self, d):
        print("+++++++++++++++++++")
        print(f"manufacturer: {d.manufacturer}")
        print(f"product: {d.product}")
        print(f"serial: {d.serial}")

    def __print_hid(self, d):
        print('--------------')
        keys = list(d.keys())
        keys.sort()
        for key in keys:
            print(f"key: {key} {d[key]}")
