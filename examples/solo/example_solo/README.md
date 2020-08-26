# 通过串口命令配置阿里云解决方案

### 介绍
`Example Solo` 为客户提供串口命令配置模组连接阿里云平台, 即可以:  
- 支持通过命令设置一键配网（零配）或者设备热点配网
- 支持通过命令使模组恢复出厂设置
- 支持通过命令配置模组四元组信息
- 支持通过命令擦除动态注册的 DeviceSecret 信息
- 支持通过命令重启模组
- 支持通过命令配置模组无线 SSID 和 Password

另外该示例还支持如下功能：
- 支持阿里<云智能> APP 控制设备
- 支持阿里 <天猫精灵智能音箱> 控制设备
- 支持阿里 <天猫精灵智能音箱> 配网并控制设备
- 支持 LED 控制(开关)
- 支持 OTA 升级

### 解决方案部署
#### 1.参考 [README](../../../README.md) 文档进行硬件准备、环境搭建、SDK 准备

#### 2.阿里云平台部署
参考示例 `smart_light` 进行阿里云平台部署，获取到四元组信息，无需烧录四元组。

#### 3.下载本工程
   ```
    git clone https://github.com/espressif/esp-aliyun.git
    cd esp-aliyun
   ```

#### 4.编译 `example solo` 并烧录运行
```
cd examples/solo/example_solo
make chip=esp32 defconfig 或者 make chip=esp8266 defconfig
make -j8 flash monitor
```

#### 5.命令手册

```
#配置四元组信息
linkkey <ProductKey> <DeviceName> <DeviceSecret> <ProductSecret>
示例：linkkey a1QdKm5axuO test_006 J4R9yh47YjAHdcIIo6M7P01MSfErusWG sUBuz3XD13sP083P
注意：顺序不能颠倒，每个参数之间使用空格隔开。如果不想设置其中的某一个，请用空格代替该参数值。
      设备第一次上电后，必须先配置四元组信息。
```

```
#配置一键配网，零配
active_awss
注意：设备默认是一键配网，配网有效时间为60s, 配网过期后，可以执行该命令重新进行配网。
```

```
#配置设备热点配网
dev_ap
注意：设备热点配网无有时间，可以一直配网，如果要切换一键配网，执行 active_awss。
```

```
#绕过配网配置无线 SSID 和 Password
netmgr connect <SSID> <Password>
示例： netmgr connect myssid 12345678
```

```
#恢复出厂设置
reset
注意：恢复出厂设置只会清除配网信息，不会擦除四元组信息。
```

```
#重启模组
reboot
注意：重启模组不会清除配网信息，不会擦除四元组信息。
```

```
#擦除动态注册的DeviceSecret
kv_clear
注意：该功能是为了动态注册后的模组重新写入其他四元组后可以正常使用，而不需要擦写模组。
```

#### 6.串口配置

||ESP32|ESP8266|
|:-----:|:-----:|:-----:|
|CMD_RX|RXD|RXD|
|CMD_TX|TXD|TXD|
|LOG_RX|RXD|RXD|
|LOG_TX|TXD|TXD|
|BAUD_RATE|115200|74880|

按照上表PIN脚接入对应串口，VCC 3.3V, GND, TX, RX。

#### 7.设备第一次运行时

配置四元组信息

```
[17:04:50.249][0;32mI (367) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE[0m
[17:04:50.249][0;32mI (367) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE[0m
[17:04:50.260]I (397) wifi:wifi firmware version: 29701b4
[17:04:50.282]I (397) wifi:config NVS flash: enabled
[17:04:50.282]I (397) wifi:config nano formating: disabled
[17:04:50.293]I (397) wifi:Init dynamic tx buffer num: 32
[17:04:50.293]I (407) wifi:Init data frame dynamic rx buffer num: 32
[17:04:50.293]I (407) wifi:Init management frame dynamic rx buffer num: 32
[17:04:50.301]I (417) wifi:Init management short buffer num: 32
[17:04:50.310]I (417) wifi:Init static rx buffer size: 1600
[17:04:50.310]I (427) wifi:Init static rx buffer num: 10
[17:04:50.310]I (427) wifi:Init dynamic rx buffer num: 32
[17:04:50.322][0;32mI (567) phy: phy_version: 4180, cb3948e, Sep 12 2019, 16:39:13, 0, 0[0m
[17:04:50.459]I (567) wifi:mode : sta (24:0a:c4:d6:d6:60)
[17:04:50.459][prt] log level set as: [ 4 ]
[17:04:50.459][0;32mI (567) uart: queue free spaces: 1[0m
[17:04:50.459]....................................................
[17:04:50.482]          DeviceName : 
[17:04:50.482]        DeviceSecret : 
[17:04:50.482]          ProductKey : 
[17:04:50.482]       ProductSecret : 
[17:04:50.482]....................................................
[17:04:50.482][0;31mE (597) app_entry: Please first input four config[0m
[17:04:50.482]
[17:07:48.598][0;32mI (178717) app_entry: ProductKey: a1QdKm5axuO[0m
[17:07:48.598][0;32mI (178717) app_entry: DeviceName: test_006[0m
[17:07:48.598][0;32mI (178717) app_entry: DeviceSecret: J4R9yh47YjAHdcIIo6M7P01MSfErusWG[0m
[17:07:48.612][0;32mI (178727) app_entry: ProductSecret: sUBuz3XD13sP083P[0m
[17:07:48.612]
```

串口执行：`linkkey a1QdKm5axuO test_006 J4R9yh47YjAHdcIIo6M7P01MSfErusWG sUBuz3XD13sP083P`

配置设备热点配网

```
[17:07:48.598][0;32mI (178717) app_entry: ProductKey: a1QdKm5axuO[0m
[17:07:48.598][0;32mI (178717) app_entry: DeviceName: test_006[0m
[17:07:48.598][0;32mI (178717) app_entry: DeviceSecret: J4R9yh47YjAHdcIIo6M7P01MSfErusWG[0m
[17:07:48.612][0;32mI (178727) app_entry: ProductSecret: sUBuz3XD13sP083P[0m
[17:07:48.612][0;33mW (217177) wrapper_kv: nvs get blob stassid failed with 1102[0m
[17:08:27.062][0;32mI (217177) app_entry: Set softap config[0m
[17:08:27.062][0;33mW (217177) wrapper_kv: nvs get blob stassid failed with 1102[0m
[17:08:27.068][1;31m[crt] awss_config_press(180): enable awss
[17:08:27.068][0m[0;32mI (217187) app main: IOTX_AWSS_ENABLE[0m
[17:08:27.080][1;31m[crt] awss_dev_ap_setup(36): ssid:adh_a1QdKm5axuO_D6D660
[17:08:27.080][0m[0;32mI (217197) awss: ssid: adh_a1QdKm5axuO_D6D660[0m
[17:08:27.089]I (217197) wifi:mode : softAP (24:0a:c4:d6:d6:61)
[17:08:27.089]I (217207) wifi:Total power save buffer number: 16
[17:08:27.101]I (217207) wifi:Init max length of beacon: 752/752
[17:08:27.101]I (217217) wifi:Init max length of beacon: 752/752
[17:08:27.101]I (217217) wifi:Total power save buffer number: 16
[17:08:27.109][0;32mI (218227) udp: success to establish udp, fd=54[0m
[17:08:28.104]
```

串口执行：`dev_ap`

#### 8.手机从[阿里云官网](https://living.aliyun.com/doc#muti-app.html) 下载 `云智能` 公版 APP, 国内用户版.

#### 9.注册好账号后,进入 APP.

如果是新注册的四元组信息，支持手机 APP 上点击 `+` 号，点击手动添加，选择灯（Wi-Fi）,进行设备热点配网。
如果是老的四元组信息，参考 `smart_light` 示例里 README 关于配网流程。

#### 11.重新配网

```
[17:22:00.499][0m[0;32mI (385657) conn_mgr: Found ssid 105[0m
[17:22:10.557][0;32mI (385677) app_entry: Reset and unbind device[0m
[17:22:10.577][1;33m[inf] wrapper_mqtt_subscribe(2879): mqtt subscribe packet sent,topic = /sys/a1QdKm5axuO/test_006/thing/reset_reply!
[17:22:10.600][0m[1;33m[inf] iotx_mc_deliver_message(1280): NO matching any topic, call default handle function
[17:22:10.781][0m[1;35m[wrn] iotx_cloud_conn_mqtt_event_handle(178): bypass 64 bytes on [/sys/a1QdKm5axuO/test_006/thing/reset]
[17:22:10.781][0m[1;33m[inf] iotx_mc_deliver_message(1280): NO matching any topic, call default handle function
[17:22:11.160][0m[1;35m[wrn] iotx_cloud_conn_mqtt_event_handle(178): bypass 162 bytes on [/sys/a1QdKm5axuO/test_006/_thing/event/notify]
[17:22:11.160][0mI (387827) wifi:state: run -> init (0)
[17:22:12.726]I (387827) wifi:pm stop, total sleep time: 272515941 us / 347673231 us
[17:22:12.726]
[17:22:12.726]I (387827) wifi:new:<7,0>, old:<7,0>, ap:<255,255>, sta:<7,0>, prof:1
[17:22:12.742][0;31mE (387837) esp-tls: read error :-76:[0m
[17:22:12.742][0;31mE (387837) conn_mgr: Disconnect reason : 8[0m
[17:22:12.742][0;31mE (387837) iot_import_tls: esp_tls_conn_read error, errno:Software caused connection abort[0m
[17:22:12.764][1;31m[err] iotx_mc_read_packet(541): mqtt read error, rc=-76
[17:22:12.764][0m[1;31m[err] iotx_mc_cycle(1508): readPacket error,result = -14
[17:22:12.764][0m[1;31m[err] _mqtt_cycle(1597): error occur rc=-14
[17:22:12.776][0m[1;31m[err] _mqtt_cycle(1597): error occur rc=-27

[17:22:12.787][0m[1;31m[err] _mqtt_cycle(1597): error occur rc=-27
[17:22:12.787][0m[1;31m[err] _mqtt_cycle(1597): error occur rc=-27
[17:22:12.787][0mI (387897) wifi:flush txq
[17:22:12.793]I (387897) wifi:stop sw txq
[17:22:12.793]I (387897) wifi:lmac stop hw txq
[17:22:12.805]ets Jun  8 2016 00:22:57
[17:22:12.805]
[17:22:12.805]rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
[17:22:12.805]configsip: 0, SPIWP:0xee
[17:22:12.814]clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
[17:22:12.814]mode:DIO, clock div:2
[17:22:12.814]load:0x3fff0018,len:4
[17:22:12.814]load:0x3fff001c,len:6820
[17:22:12.825]load:0x40078000,len:12072
[17:22:12.825]load:0x40080400,len:6708
[17:22:12.825]entry 0x40080778
[17:22:12.825][0;32mI (71) boot: Chip Revision: 1[0m
[17:22:12.835][0;32mI (72) boot_comm: chip revision: 1, min. bootloader chip revision: 0[0m
[17:22:12.844][0;32mI (39) boot: ESP-IDF v3.3.2-1-g31678d5-dirty 2nd stage bootloader[0m
[17:22:12.844][0;32mI (39) boot: compile time 14:30:17[0m
[17:22:12.855][0;32mI (40) boot: Enabling RNG early entropy source...[0m
[17:22:12.855][0;32mI (46) boot: SPI Speed      : 40MHz[0m
[17:22:12.855][0;32mI (50) boot: SPI Mode       : DIO[0m
[17:22:12.865][0;32mI (54) boot: SPI Flash Size : 4MB[0m
[17:22:12.875][0;32mI (58) boot: Partition Table:[0m
[17:22:12.875][0;32mI (62) boot: ## Label            Usage          Type ST Offset   Length[0m
[17:22:12.884][0;32mI (69) boot:  0 nvs              WiFi data        01 02 00009000 00004000[0m
[17:22:12.884][0;32mI (76) boot:  1 otadata          OTA data         01 00 0000d000 00002000[0m
[17:22:12.894][0;32mI (84) boot:  2 phy_init         RF data          01 01 0000f000 00001000[0m
[17:22:12.905][0;32mI (91) boot:  3 ota_0            OTA app          00 10 00010000 00100000[0m
[17:22:12.905][0;32mI (99) boot:  4 ota_1            OTA app          00 11 00110000 00100000[0m
[17:22:12.922][0;32mI (106) boot:  5 fctry            WiFi data        01 02 00210000 00004000[0m
[17:22:12.922][0;32mI (114) boot: End of partition table[0m
[17:22:12.933][0;32mI (118) boot_comm: chip revision: 1, min. application chip revision: 0[0m
[17:22:12.933][0;32mI (125) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f400020 size=0x2b768 (178024) map[0m
[17:22:12.944][0;32mI (197) esp_image: segment 1: paddr=0x0003b790 vaddr=0x3ffb0000 size=0x03944 ( 14660) load[0m
[17:22:13.002][0;32mI (203) esp_image: segment 2: paddr=0x0003f0dc vaddr=0x40080000 size=0x00400 (  1024) load[0m
[17:22:13.014][0;32mI (204) esp_image: segment 3: paddr=0x0003f4e4 vaddr=0x40080400 size=0x00b2c (  2860) load[0m
[17:22:13.025][0;32mI (214) esp_image: segment 4: paddr=0x00040018 vaddr=0x400d0018 size=0xabc54 (703572) map[0m
[17:22:13.034][0;32mI (468) esp_image: segment 5: paddr=0x000ebc74 vaddr=0x40080f2c size=0x143f0 ( 82928) load[0m
[17:22:13.273][0;32mI (515) boot: Loaded app from partition at offset 0x10000[0m
[17:22:13.326][0;32mI (515) boot: Disabling RNG early entropy source...[0m
[17:22:13.326][0;32mI (516) cpu_start: Pro cpu up.[0m
[17:22:13.326][0;32mI (519) cpu_start: Application information:[0m
[17:22:13.335][0;32mI (524) cpu_start: Project name:     example_solo[0m
[17:22:13.345][0;32mI (530) cpu_start: App version:      v1.0-322-gc4f8f5a-dirty[0m
[17:22:13.345][0;32mI (536) cpu_start: Compile time:     Aug  3 2020 14:39:15[0m
[17:22:13.355][0;32mI (542) cpu_start: ELF file SHA256:  95d66c59c0ba4c18...[0m
[17:22:13.355][0;32mI (548) cpu_start: ESP-IDF:          v3.3.2-1-g31678d5-dirty[0m
[17:22:13.365][0;32mI (554) cpu_start: Starting app cpu, entry point is 0x400811d8[0m
[17:22:13.376][0;32mI (541) cpu_start: App cpu up.[0m
[17:22:13.376][0;32mI (565) heap_init: Initializing. RAM available for dynamic allocation:[0m
[17:22:13.383][0;32mI (572) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM[0m
[17:22:13.395][0;32mI (578) heap_init: At 3FFB9E38 len 000261C8 (152 KiB): DRAM[0m
[17:22:13.395][0;32mI (584) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM[0m
[17:22:13.404][0;32mI (591) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM[0m
[17:22:13.404][0;32mI (597) heap_init: At 4009531C len 0000ACE4 (43 KiB): IRAM[0m
[17:22:13.415][0;32mI (603) cpu_start: Pro cpu start user code[0m
[17:22:13.415][0;32mI (286) cpu_start: Starting scheduler on PRO CPU.[0m
[17:22:13.425][0;32mI (0) cpu_start: Starting scheduler on APP CPU.[0m
[17:22:13.434]I (367) wifi:wifi driver task: 3ffc1de4, prio:23, stack:3584, core=0
[17:22:13.474][0;32mI (367) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE[0m
[17:22:13.474][0;32mI (367) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE[0m
[17:22:13.484]I (397) wifi:wifi firmware version: 29701b4
[17:22:13.502]I (397) wifi:config NVS flash: enabled
[17:22:13.513]I (397) wifi:config nano formating: disabled
[17:22:13.513]I (397) wifi:Init dynamic tx buffer num: 32
[17:22:13.513]I (407) wifi:Init data frame dynamic rx buffer num: 32
[17:22:13.526]I (407) wifi:Init management frame dynamic rx buffer num: 32
[17:22:13.526]I (417) wifi:Init management short buffer num: 32
[17:22:13.535]I (417) wifi:Init static rx buffer size: 1600
[17:22:13.535]I (427) wifi:Init static rx buffer num: 10
[17:22:13.535]I (427) wifi:Init dynamic rx buffer num: 32
[17:22:13.544][0;32mI (547) phy: phy_version: 4180, cb3948e, Sep 12 2019, 16:39:13, 0, 0[0m
[17:22:13.651]I (547) wifi:mode : sta (24:0a:c4:d6:d6:60)
[17:22:13.651][prt] log level set as: [ 4 ]
[17:22:13.665][0;32mI (547) uart: queue free spaces: 1[0m
[17:22:13.665]....................................................
[17:22:13.675]          DeviceName : test_006
[17:22:13.675]        DeviceSecret : J4R9yh47YjAHdcIIo6M7P01MSfErusWG
[17:22:13.675]          ProductKey : a1QdKm5axuO
[17:22:13.685]       ProductSecret : sUBuz3XD13sP083P
[17:22:13.685]....................................................
[17:22:13.693][0;33mW (577) wrapper_kv: nvs get blob stassid failed with 1102[0m
[17:22:13.693][1;31m[crt] awss_config_press(180): enable awss
[17:22:13.703][0m[0;32mI (587) app main: IOTX_AWSS_ENABLE[0m
[17:22:13.703][1;31m[crt] awss_dev_ap_setup(36): ssid:adh_a1QdKm5axuO_D6D660
[17:22:13.714][0m[0;32mI (597) awss: ssid: adh_a1QdKm5axuO_D6D660[0m
[17:22:13.714]I (607) wifi:mode : softAP (24:0a:c4:d6:d6:61)
[17:22:13.714]I (607) wifi:Total power save buffer number: 16
[17:22:13.726]I (617) wifi:Init max length of beacon: 752/752
[17:22:13.726]I (617) wifi:Init max length of beacon: 752/752
[17:22:13.733]I (627) wifi:Total power save buffer number: 16
[17:22:13.733][0;32mI (1627) udp: success to establish udp, fd=54[0m
[17:22:14.728]
```
串口执行： `reset`

#### 12.OTA 支持
参考 examples/ota/ota_example_mqtt 示例下的 [README](../../ota/ota_example_mqtt/README.md) , 向管理控制台上传固件, 验证固件后, 下发升级指令.
设备端收到升级指令后, 即开始 OTA.

升级完成后, 会检查固件的有效性.

#### 13. 其他相关功能可参考 `smart_light` 示例README