# ESP 设备对接阿里云指南
# 目录

- [0.介绍](#Introduction)  
- [1.目的](#aim)  
- [2.硬件准备](#hardwareprepare)  
- [3.阿里云平台准备](#aliyunprepare)  
- [4.环境搭建](#compileprepare)  
- [5.SDK 准备](#sdkprepare)  
- [6.编译&烧写&运行](#makeflash)  

# <span id = "Introduction">0.介绍</span>
[乐鑫](https://www.espressif.com/zh-hans)是高集成度芯片的设计专家，专注于设计简单灵活、易于制造和部署的解决方案。乐鑫研发和设计 IoT 业内集成度高、性能稳定、功耗低的无线系统级芯片，乐鑫的模组产品集成了自主研发的系统级芯片，因此具备强大的 Wi-Fi 和蓝牙功能，以及出色的射频性能。

[阿里云物联网套件](https://github.com/aliyun/iotkit-embedded)是阿里云专门为物联网领域的开发人员推出的，其目的是帮助开发者搭建安全性能强大的数据通道，方便终端（如传感器、执行器、嵌入式设备或智能家电等等）和云端的双向通信。全球多节点部署让海量设备全球范围都可以安全低延时接入阿里云IoT Hub，安全上提供多重防护保障设备云端安全，性能上能够支撑亿级设备长连接，百万消息并发。物联网套件还提供了一站式托管服务，数据从采集到计算到存储，用户无需购买服务器部署分布式架构，用户通过规则引擎只需在web上配置规则即可实现采集+计算+存储等全栈服务。总而言之，基于物联网套件提供的服务，物联网开发者可以快速搭建稳定可靠的物联网平台。

# <span id = "aim">1.目的</span>
本文基于 linux 环境，介绍 ESP 设备对接阿里云平台的具体流程，供读者参考。当前只维护 **smart_light** 和 **solo** 示例，这两个示例包含了另外三个示例的功能，建议直接选择这两个示例 demo。

# <span id = "hardwareprepare">2.硬件准备</span>
- **linux 环境**  
用来编译 & 烧写 & 运行等操作的必须环境。 
> windows 用户可安装虚拟机，在虚拟机中安装 linux。

- **ESP 设备**  
ESP 设备包括 [ESP芯片](https://www.espressif.com/zh-hans/products/hardware/socs)，[ESP模组](https://www.espressif.com/zh-hans/products/hardware/modules)，[ESP开发板](https://www.espressif.com/zh-hans/products/hardware/development-boards)等。

- **USB 线**  
连接 PC 和 ESP 设备，用来烧写/下载程序，查看 log 等。

# <span id = "aliyunprepare">3.阿里云平台准备</span>
根据[阿里官方文档](https://github.com/aliyun/iotkit-embedded?spm=5176.doc42648.2.4.e9Zu05)，在阿里云平台创建产品，创建设备，同时自动产生 `product key`, `product secert`, `device name`, `device secret`。  
`product key`, `product secert`, `device name`, `device secret` 将在 6.2.3 节用到。

# <span id = "compileprepare">4.环境搭建</span>
**如果您熟悉 ESP 开发环境，可以很顺利理解下面步骤; 如果您不熟悉某个部分，比如编译，烧录，需要您结合官方的相关文档来理解。如您需阅读 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/index.html)文档等。**  

## 4.1 编译器环境搭建
- ESP8266 平台: 根据[官方链接](https://github.com/espressif/ESP8266_RTOS_SDK)中 **Get toolchain**，获取 toolchain
- ESP32  & ESP32S2 平台：根据[官方链接](https://github.com/espressif/esp-idf/blob/master/docs/zh_CN/get-started/linux-setup.rst)中 **工具链的设置**，下载 toolchain

toolchain 设置参考 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/get-started/index.html#get-started-setup-toolchain)。  
## 4.2 烧录工具/下载工具获取
- ESP8266 平台：烧录工具位于 [ESP8266_RTOS_SDK](https://github.com/espressif/ESP8266_RTOS_SDK) 下 `./components/esptool_py/esptool/esptool.py`
- ESP32 & ESP32S2 平台：烧录工具位于 [esp-idf](https://github.com/espressif/esp-idf) 下 `./components/esptool_py/esptool/esptool.py`

esptool 功能参考:  

```
$ ./components/esptool_py/esptool/esptool.py --help
```

# <span id = "sdkprepare">5.SDK 准备</span> 
- [esp-aliyun SDK](https://github.com/espressif/esp-aliyun), 通过该 SDK 可实现使用 MQTT 协议，连接 ESP 设备到阿里云。
- Espressif SDK
  - ESP32 & ESP32S2 平台: [ESP-IDF](https://github.com/espressif/esp-idf)
  - ESP8266 平台: [ESP8266_RTOS_SDK](https://github.com/espressif/ESP8266_RTOS_SDK)

> Espressif SDK 下载好后：  
> ESP-IDF: 请切换到 v4.2 分支： `git checkout v4.2`
如果需要使用 ESP32S2 模组，请切换到 v4.2 版本： `git checkout v4.2`
> ESP8266_RTOS_SDK: 请切换到 v3.3 分支： `git checkout v3.3`

# <span id = "makeflash">6.编译 & 烧写 & 运行</span>
## 6.1 编译

### 6.1.1 导出编译器
参考 [工具链的设置](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/get-started/linux-setup.html)

### 6.1.2 编译 demo 示例
**由于 esp32 和 esp8266 将会采用不同的 sdkconfig.defaults 和对应的 partitions.csv，在对应的 make 命令中加入了对应的芯片选项，如 chip=esp32 或 chip=esp8266。**

当 chip=esp32 时将默认使用 sdkconfig_esp32.defaults 以及 partitions_esp32.csv。

当 chip=esp8266 时将默认使用 sdkconfig_esp8266.defaults 以及 partitions_esp8266.csv。

当使用 esp32s2 时，将默认使用 sdkconfig.defaults ，sdkconfig.defaults.esp32s2 以及 partitions_esp32s2.csv，编译方式与 8266 & 32 都不一样，需要使用 cmake 进行编译。

以上需要特别注意。

在 esp-aliyun 目录下执行：

```
cd examples/solutions/smart_light
make chip=esp32 defconfig
make menuconfig
```

如果需要编译esp32s2版本, 请按照如下步骤编译:

执行如下命令，以 solo 示例为例，目前只支持 solo 和 smart_light 示例。

```
cd examples/solo/example_solo
idf.py set-target esp32s2
idf.py menuconfig
```

![p1](docs/_static/p1.png)

- 配置烧写串口
- 配置 `WIFI_SSID`, `WIFI_PASSWORD`

2.生成最终 bin

```
make -j8
```
使用 esp32s2 生成 bin

```
idf.py build
```

## 6.2 擦除 & 编译烧写 & 下载固件 & 查看 log
将 USB 线连接好 ESP 设备和 PC,确保烧写端口正确。 

### 6.2.1[可选] 擦除 flash
```
make erase_flash
```
> 注：无需每次擦除，擦除后需要重做 6.2.3。

### 6.2.2 烧录程序
```
make flash
```

使用 esp32s2 擦除 flash
```
idf.py -p (PORT) erase_flash
```

### 6.2.3 烧录三元组信息
参考 [量产说明](./config/mass_mfg/README.md) 文档烧录三元组 NVS 分区。

## 6.2.4 运行

```
make monitor
```

如将 ESP8266 拨至运行状态，即可看到如下 log：
log 显示了 ESP8266 基于 TLS 建立了与阿里云的安全连接通路，接着通过 MQTT 协议订阅和发布消息，同时在阿里云控制台上，也能看到 ESP8266 推送的 MQTT 消息。  

![p2](docs/_static/p2.png)

![p3](docs/_static/p3.png)

![p4](docs/_static/p4.png)

> 也可执行 `make flash monitor` 来编译烧写和查看 log。
