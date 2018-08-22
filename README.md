# esp32-aliyun-demo  

## 1. 阿里云物联网套件简介

![](https://i.imgur.com/OqjGiQL.png)

阿里云物联网套件包括以下部分：  

> - IoT Hub  
>  为设备和物联网应用程序提供发布和接收消息的安全通道。
>   详情请参考 [IoT Hub](https://help.aliyun.com/document_detail/30548.html?spm=5176.doc30523.2.1.WtHk0t)
> - 安全认证&权限策略
> - 规则引擎
> - 设备影子

**esp32-aliyun-demo 移植下载:**  

    git clone --recursive https://github.com/espressif/esp32-aliyun-demo.git  
    git submodule update --init --recursive

## 2. framework  

```
esp32-aliyun-demo                   // 工程根目录
├── components                      // 核心移植组件
│      └── esp32-aliyun             // esp32-aliyun 组件
|           ├── component.mk        // 适配层 platform 编译配置
|           ├── config              // 阿里云编译配置
|           ├── iotkit-embedded     // 阿里云 SDK
|           ├── iotkit-embedded.pkgs    // 阿里云编译备份
|           ├── Kconfig             // 配置阿里云选项
|           ├── Makefile.projbuild  // 编译阿里云 SDK 入口
|           └── platform            // 阿里云适配层
├── esp-idf                         // esp-idf
├── examples                        // sample 入口
|    └── mqtt                       // mqtt demo
├── README.md                       // 说明文档
└── setup_env.sh                    // 一键搭建编译环境脚本(linux)
```  
在用户进行开发时：  
所有和 `iotkit-embedded` 中相关的功能函数均只需要调用 `iot_export.h` 和 `iot_import.h`两个头文件，无须关心其他头文件；  
所有和 `esp-idf` 相关的功能函数需要参考 `esp-idf` 具体实现。  

## 3. 硬件平台  

- 开发板：[ESP32-DevKitC 开发板](http://esp-idf.readthedocs.io/en/latest/hw-reference/modules-and-boards.html#esp32-core-board-v2-esp32-devkitc) 或 [ESP-WROVER-KIT 开发板](http://esp-idf.readthedocs.io/en/latest/hw-reference/modules-and-boards.html#esp-wrover-kit)

- 路由器/ Wi-Fi 热点：可以连接外网

## 4. 编译环境搭建( ubuntu 16.04)  

- 如果是在 ubuntu x64 下进行开发，则只需要在 PC 有网络连接时运行 `set_env.sh` 脚本一键完成编译环境的搭建。  

- 本 Demo 适配于 [esp-idf](https://github.com/espressif/esp-idf), 如果已经在开发环境下对 esp-idf 进行过配置，则无须运行 `set_env.sh` 脚本。
 

- 如果是在其他平台下进行开发，具体请参考 [Get Started](http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

## 5. 配置 && 编译 && 烧写 && 运行
进入 examples 目录下的 子目录。
#### 5.1 编译阿里云 SDK
```
$ make aliyun
```

#### 5.2 配置 demo
运行 `make menuconfig` -> Demo Configuration 配置如下选项
- Wifi ssid
- Wifi passwd
- product key
- device name
- device secret

#### 5.3 编译烧写运行
```
$ make flash monitor
```








