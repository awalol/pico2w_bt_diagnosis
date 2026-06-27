# pico2w_bt_diagnosis

[English](README.md)

这是一个用于 Raspberry Pi Pico 2 W 的最小化 Bluetooth Classic inquiry 诊断固件。

程序会启动 Pico 2 W 的 CYW43 蓝牙控制器，执行蓝牙 inquiry 扫描，通过 USB 串口打印附近设备信息，并每隔几秒自动重新扫描。它主要用于判断 Pico 2 W 的蓝牙初始化和扫描功能是否正常。

## 输出内容

USB 串口日志会包含：

- 板卡和 Pico SDK 信息
- CYW43 初始化状态
- 蓝牙 ready 状态
- 扫描轮次
- 扫描到的蓝牙设备地址
- 设备类型 Class of Device
- RSSI，如果可用
- 设备名称，如果可用

## 环境要求

- Raspberry Pi Pico 2 W
- Raspberry Pi Pico SDK 2.2.0 或兼容版本
- CMake 和 Ninja
- USB 串口监视器

项目当前配置为：

```cmake
set(PICO_BOARD pico2_w CACHE STRING "Board type")
```

## 构建

在项目目录运行：

```powershell
cmake --build .\build
```

主要 UF2 输出文件是：

```text
build/pico2w_bt_diagnosis.uf2
```

如果 `build` 目录还没有配置过，请先在 Pico SDK 环境中执行：

```powershell
cmake -S . -B build -G Ninja
cmake --build .\build
```

## 烧录

按住 Pico 2 W 的 BOOTSEL 按钮并插入 USB，然后把下面的文件复制到挂载出来的 `RP2350` 盘：

```text
build/pico2w_bt_diagnosis.uf2
```

## 串口日志

烧录后打开 Pico 的 USB 串口。固件会短暂等待 USB 串口连接，即使没有打开串口监视器也会继续运行。

示例输出：

```text
pico2w_bt_diagnosis
Board: pico2_w
Pico SDK: 2.2.0
CYW43 init status: 0
Powering on Bluetooth...
Bluetooth ready

Scan 1 started
[1] device 1: addr=AA:BB:CC:DD:EE:FF, cod=0x240404, rssi=-52 dBm, name="Device"
Scan 1 complete: 1 device(s)
Next scan in 5000 ms.
```

## 说明

- 已启用 USB 串口输出，禁用 UART stdio。
- 固件使用 BTstack 的 Bluetooth Classic inquiry。
- 这是诊断固件，不是蓝牙配对或 HID 连接示例。
