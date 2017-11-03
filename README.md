# 基于 SSL/TLS 的 MQTT 参考示例 
---------------------------------------------------------------
### 一: 目的 
##### 1. 将 ESP8266 上收集到的数据通过基于阿里物联网套件的 MQTT 协议发送到阿里物联网控制台  
##### 2. ESP8266 通过订阅来接收来自阿里物联网控制台发布的 MQTT 消息  
  
  
### 二: 前提准备
##### 1. 一颗来自于乐鑫科技的精致WiFi芯片/模组- ESP8266/ESP-WROOM-02
##### 2. 编译器和编译环境搭建，完成在 8266 上第一个 hello-world.[Click](http://www.espressif.com/sites/default/files/documentation/2a-esp8266-sdk_getting_started_guide_cn.pdf)
  
  
### 三: 示例步骤
#### Step 1: 阿里物联网控制台环境搭建
  
#### a) [创建/登录阿里云账号](https://account.aliyun.com/login/login.htm)
#### b) 登录账号后，左上角全部导航->产品->物联网->物联网套件，开通并管理控制台
  
在控制台创建产品，如 ESP8266，创建完后，会得到一个 ProductKey，这是我们应用和控制台交互中必要的访问参数
![如图](https://github.com/ustccw/RepoForShareData/blob/master/Alibaba/Hemashengxian/pic/%E4%BA%A7%E5%93%81%E7%AE%A1%E7%90%86.png)
  
进入产品中的设备管理，可以添加设备等，如我们创建设备 esp8266_test001，我们或获取到 deviceName 和 deviceSecret，这也是我们应用和控制台交互中必要的访问参数
![如图](https://github.com/ustccw/RepoForShareData/blob/master/Alibaba/Hemashengxian/pic/%E8%AE%BE%E5%A4%87%E7%AE%A1%E7%90%86.png)

#### c) 在消息通信页面设置一些 MQTT 的 Topic,如我们通过定义Topic类来定义 relay 和 data 的 topic
![如图](https://github.com/ustccw/RepoForShareData/blob/master/Alibaba/Hemashengxian/pic/topic.png)

#### d)[可选] 开通**规则引擎**,目的是转发 MQTT 消息  
  
首先创建规则:

![如图](https://github.com/ustccw/RepoForShareData/blob/master/Alibaba/Hemashengxian/pic/%E8%A7%84%E5%88%99%E5%BC%95%E6%93%8E-%E5%88%9B%E5%BB%BA%E8%A7%84%E5%88%99.png)
  
其次建立转发机制:
![如图](https://github.com/ustccw/RepoForShareData/blob/master/Alibaba/Hemashengxian/pic/%E8%A7%84%E5%88%99%E5%BC%95%E6%93%8E-%E8%BD%AC%E5%8F%91%E8%AF%A6%E7%BB%86.png)
  
#### Step 2: 编译&烧写
**获取本仓库之后，需要做一些小的SDK改动来适应你的开发环境**
  
#### a) 修改产品信息
  
#### 在 user/user_main.c 中修改以下字段
```
PRODUCT_KEY
DEVICE_NAME
DEVICE_SECRET

TOPIC_DATA
TOPIC_UPDATE
TOPIC_ERROR
TOPIC_GET
TOPIC_RELAY
```

#### b)[可选] sntp server 修改

在 user/user_main.c 中修改 sntp_setservername 使得 SNTP 可以正常启动

#### c) 修改 WiFi 信息

在 initialize_wifi() 中 修改你的 WiFi SSID 和 WiFi password

#### d) 编译

```
$./gen_misc.sh
```
 
#### e) 擦除整块 flash 

**将 ESP8266 拨至烧写状态**

```
$~/esp/esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 --baud 921600 erase_flash
```

#### f) 烧写固件 

**将 ESP8266 拨至烧写状态**

```
$python ~/esp/esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash --flash_size 2MB-c1 0x00000 ../glab_esp8266-aliyun-demo/bin/boot_v1.6.bin 0x1000 ../glab_esp8266-aliyun-demo/bin/upgrade/user1.2048.new.5.bin 0x1fe000 ../glab_esp8266-aliyun-demo/bin/blank.bin 0x1fc000 ../glab_esp8266-aliyun-demo/bin/esp_init_data_default.bin
```

#### Step 3: 运行&串口显示结果

**将 ESP8266 拨至运行状态**

```
$miniterm.py /dev/ttyUSB0 74880
```
**如果以上正常进行，您将看到 ESP8266 上传 data ，同时 ESP8266 收到来自控制台的 data ,以及规则引擎转发的 relay 。**

log大致如下:

ets Jan  8 2013,rst cause:1, boot mode:(3,2)

load 0x40100000, len 2408, room 16 

tail 8

chksum 0xe5

load 0x3ffe8000, len 776, room 0 

tail 8

chksum 0x84

load 0x3ffe8310, len 632, room 0 

tail 8

chksum 0xd8

csum 0xd8

2nd boot version : 1.6

  SPI Speed      : 40MHz

  SPI Mode       : QIO

  SPI Flash Size & Map: 16Mbit(1024KB+1024KB)

jump to run user1 @ 1000



OS SDK ver: 1.5.0-dev(caff253) compiled @ Oct 23 2017 17:42:20

phy ver: 1055_1, pp ver: 10.7



rf cal sector: 507

tcpip_task_hdl : 3ffeffd0, prio:10,stack:512

idle_task_hdl : 3fff0070,prio:0, stack:384

tim_task_hdl : 3fff2828, prio:2,stack:512

SDK version:1.5.0-dev(caff253) 

SDK compile time:Oct 24 2017 20:04:39

SDK version:1.5.0-dev(caff253) 50840

mode : sta(18:fe:34:ed:87:fc)

add if0

[heap check task] free heap size:49064

scandone

state: 0 -> 2 (b0)

state: 2 -> 3 (0)

state: 3 -> 5 (10)

add 0

aid 7

pm open phy_2,type:2 0 0

connected with BL_841R, channel 12

dhcp client start...

cnt 

[heap check task] free heap size:48288

ip:192.168.111.105,mask:255.255.255.0,gw:192.168.111.1

Connected.Initializing SNTP

please start sntp first !

current time : Thu Jan 01 00:00:00 1970

did not get a valid time from sntp server

[heap check task] free heap size:48296

current time : Tue Oct 24 20:05:01 2017

[ALIYUN] MQTT client example begin, free heap size:40016

[inf] _ssl_client_init(166): Loading the CA root certificate ...

cert. version     : 3

serial number     : 04:00:00:00:00:01:15:4B:5A:C3:94

issuer name       : C=BE, O=GlobalSign nv-sa, OU=Root CA, CN=GlobalSign Root CA

subject name      : C=BE, O=GlobalSign nv-sa, OU=Root CA, CN=GlobalSign Root CA

issued  on        : 1998-09-01 12:00:00

expires on        : 2028-01-28 12:00:00

signed using      : RSA with SHA1

RSA key size      : 2048 bits

basic constraints : CA=true

key usage         : Key Cert Sign, CRL Sign

[inf] _ssl_parse_crt(134): crt content:451

[inf] _ssl_client_init(174):  ok (0 skipped)

[inf] TLSConnectNetwork(340): Connecting to /iot-auth.cn-shanghai.aliyuncs.com/443...

[inf] TLSConnectNetwork(345):  ok

[inf] TLSConnectNetwork(350):   . Setting up the SSL/TLS structure...

[inf] TLSConnectNetwork(360):  ok

[inf] TLSConnectNetwork(395): Performing the SSL/TLS handshake...

[inf] TLSConnectNetwork(403):  ok

[inf] TLSConnectNetwork(407):   . Verifying peer X.509 certificate..

[inf] _real_confirm(83): certificate verification result: 0x00

[inf] utils_network_ssl_disconnect(300): ssl_disconnect

[inf] _ssl_client_init(166): Loading the CA root certificate ...

cert. version     : 3

serial number     : 04:00:00:00:00:01:15:4B:5A:C3:94

issuer name       : C=BE, O=GlobalSign nv-sa, OU=Root CA, CN=GlobalSign Root CA

subject name      : C=BE, O=GlobalSign nv-sa, OU=Root CA, CN=GlobalSign Root CA

issued  on        : 1998-09-01 12:00:00

expires on        : 2028-01-28 12:00:00

signed using      : RSA with SHA1

RSA key size      : 2048 bits


basic constraints : CA=true

key usage         : Key Cert Sign, CRL Sign

[inf] _ssl_parse_crt(134): crt content:451

[inf] _ssl_client_init(174):  ok (0 skipped)

[inf] TLSConnectNetwork(340): Connecting to /public.iot-as-mqtt.cn-shanghai.aliyuncs.com/1883...

[inf] TLSConnectNetwork(345):  ok

[inf] TLSConnectNetwork(350):   . Setting up the SSL/TLS structure...

[inf] TLSConnectNetwork(360):  ok

[inf] TLSConnectNetwork(395): Performing the SSL/TLS handshake...

[heap check task] free heap size:16512

[inf] TLSConnectNetwork(403):  ok

[inf] TLSConnectNetwork(407):   . Verifying peer X.509 certificate..

[inf] _real_confirm(83): certificate verification result: 0x00

[heap check task] free heap size:15888

mqtt_client|326 :: rc = IOT_MQTT_Publish() = 3

mqtt_client|353 :: packet-id=4, publish topic msg={"attr_name":"temperature", "attr_value":"0"}

file:mqtt_client.c function:iotx_mc_cycle line:1567 heap size:12440 type:9

SUBACK file:mqtt_client.c function:iotx_mc_cycle line:1587 heap size:12440

event_handle|138 :: subscribe success, packet-id=1

file:mqtt_client.c function:iotx_mc_cycle line:1567 heap size:12584 type:9

SUBACK file:mqtt_client.c function:iotx_mc_cycle line:1587 heap size:12584

event_handle|138 :: subscribe success, packet-id=2

file:mqtt_client.c function:iotx_mc_cycle line:1567 heap size:14304 type:4

PUBACK file:mqtt_client.c function:iotx_mc_cycle line:1577 heap size:14256

event_handle|162 :: publish success, packet-id=3

file:mqtt_client.c function:iotx_mc_cycle line:1567 heap size:16000 type:3

PUBLISH file:mqtt_client.c function:iotx_mc_cycle line:1596 heap size:15968

_demo_message_arrive|223 :: ----

_demo_message_arrive|227 :: Topic: '*s' (Length: 33)

********** topic [len:33] start addr:3fff6ab4 **********

/ymXuzyfmuQb/esp8266_test001/data

---------- topic End ----------

********** payload [len:22] start addr:3fff6ad7 **********

message: 20 hello! 20 start!

---------- payload End ----------

_demo_message_arrive|233 :: Payload: '*s' (Length: 22)

_demo_message_arrive|234 :: ----

file:mqtt_client.c function:iotx_mc_cycle line:1567 heap size:14352 type:4

PUBACK file:mqtt_client.c function:iotx_mc_cycle line:1577 heap size:14352

event_handle|162 :: publish success, packet-id=4

file:mqtt_client.c function:iotx_mc_cycle line:1567 heap size:14528 type:3

PUBLISH file:mqtt_client.c function:iotx_mc_cycle line:1596 heap size:14528

_demo_message_arrive|223 :: ----

_demo_message_arrive|227 :: Topic: '*s' (Length: 33)

********** topic [len:33] start addr:3fff6ab4 **********

/ymXuzyfmuQb/esp8266_test001/data

---------- topic End ----------

********** payload [len:45] start addr:3fff6ad7 **********

{"attr_name":"temperature", 20 "attr_value":"0"}

---------- payload End ----------

_demo_message_arrive|233 :: Payload: '*s' (Length: 45)

_demo_message_arrive|234 :: ----

file:mqtt_client.c function:iotx_mc_cycle line:1567 heap size:14568 type:3

PUBLISH file:mqtt_client.c function:iotx_mc_cycle line:1596 heap size:14568

_demo_message_arrive|223 :: ----

_demo_message_arrive|227 :: Topic: '*s' (Length: 34)

********** topic [len:34] start addr:3fff6ab4 **********

/ymXuzyfmuQb/esp8266_test001/relay

---------- topic End ----------


********** payload [len:44] start addr:3fff6ad6 **********

{"attr_name":"temperature","attr_value":"0"}

---------- payload End ----------

_demo_message_arrive|233 :: Payload: '*s' (Length: 44)

_demo_message_arrive|234 :: ----

[heap check task] free heap size:16176

mqtt_client|353 :: packet-id=5, publish topic msg={"attr_name":"temperature", "attr_value":"0"}

#### Step 4: 控制台显示结果
![如图](https://github.com/ustccw/RepoForShareData/blob/master/Alibaba/Hemashengxian/pic/consoleresult.png)

