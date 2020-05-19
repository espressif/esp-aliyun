# Smart Light 解决方案

### 介绍
`Smart Light` 为客户提供智能灯产品的解决方案. 客户几乎不需要投入软件开发, 即可以:  
- 支持阿里 <云智能> APP 一键配网
- 支持阿里 <云智能> APP 控制设备
- 支持阿里 <天猫精灵智能音箱> 控制设备
- 支持阿里 <天猫精灵智能音箱> 配网并控制设备
- 支持 LED 控制(开关,颜色等)
- 支持 OTA 升级

### 解决方案部署
#### 1.参考 [README](../../../README.md) 文档进行硬件准备、环境搭建、SDK 准备

#### 2.阿里云平台部署
在阿里云 [生活物联网平台](https://living.aliyun.com/#/) 创建产品, 参考[创建产品文档](https://living.aliyun.com/doc#readygo.html).
> 配置较多, 如果不太懂, 也不用纠结, 后续都可以修改.

部署自己的产品, 可参考如下:
新增 RGB 调色功能:
![](_static/p1.png)

新增测试设备, 此处即可以获得`三元组`, 后续需要烧录到 NVS 分区.
![](_static/p2.png)
![](_static/p3.png)

选择面板, 手机 APP 上会显示同样界面; `配网二维码`是贴在产品包装上, 终端客户给设备配网中需扫描此二维码.
![](_static/p4.png)

选择面板时, 主题面板在手机上仅能显示标准界面, 没有 RGB 调色功能. 可以自定义面板, 增加 RGB 调色.
![](_static/p5.png)

![](_static/p6.png)

配网方案选择:
![](_static/p7.png)

完成
![](_static/p8.png)

#### 3.下载本工程
   ```
    git clone https://github.com/espressif/esp-aliyun.git
    cd esp-aliyun
   ```

#### 4.烧录三元组信息
- 参考 [量产说明](../../../config/mass_mfg/README.md) 文档烧录三元组 NVS 分区.

> 如果执行了 `make erase_flash`, 需要重新烧录三元组.

#### 5.配置 `smart light example`
- RGB 灯分别接 ESP32/ESP8266 开发板上 `GPIO0`, `GPIO2`, `GPIO4` (可在 `lightbulb.c` 中修改)

#### 6.编译 `smart light` 并烧录运行
```
cd examples/solutions/smart_light
make chip=esp32 defconfig 或者 make chip=esp8266 defconfig
make -j8 flash monitor
```

> 在测试配网中, 请先执行 `make erase_flash` .

#### 7.设备第一次运行时, 会进入配网

![](_static/p9.png)

#### 8.手机从[阿里云官网](https://living.aliyun.com/doc#muti-app.html) 下载 `云智能` 公版 APP, 国内用户版.

#### 9.注册好账号后,进入 APP, 右上角扫描, 扫描第二步的二维码配网.
设备端配网成功后会保存 `ssid` 和 `password` :
![](_static/p10.png)

设备与手机绑定成功后, APP 上会弹出灯的配置页面. 返回主页显示灯 `在线`.  
![](_static/p11.png)
![](_static/p12.png)

#### 10.控制智能灯

在 APP 上打开灯, 设备端收到消息:
![](_static/p13.png)

在 APP 上设置 RGB 调色:  
![](_static/p14.png)

设备端即解析 RGB 颜色, 并设置到具体的灯产品上.
![](_static/p15.png)

#### 11.重新配网
快速重启设备 5 次, 设备会擦除配置信息, 重新进入配网状态.

可以配置快速重启的次数和超时时间.
```
cd examples/solutions/smart_light
make menuconfig
```
![](_static/p20.png)
![](_static/p21.png)

#### 12.OTA 支持
参考 examples/ota/ota_example_mqtt 示例下的 [README](../../ota/ota_example_mqtt/README.md) , 向管理控制台上传固件, 验证固件后, 下发升级指令.
设备端收到升级指令后, 即开始 OTA:
![](_static/p16.png)

升级完成后, 会检查固件的有效性, 下图说明固件有效.
![](_static/p17.png)

iotkit-embedded 目前没有设置软重启操作, 可以手动按模组重启键运行新固件:
![](_static/p18.png)

#### 13.天猫精灵
##### 13.1 天猫精灵控制设备
针对使用公版 APP 的产品，用户可以一键开通天猫精灵，实现天猫精灵音箱对设备的控制. 使用步骤参照[阿里云文档](https://living.aliyun.com/doc#TmallGenie.html).
- 在阿里云 [生活物联网平台](https://living.aliyun.com/#/)上一键开通天猫精灵, 查看功能映射.
- 在 `云智能` [公版 APP]((https://living.aliyun.com/doc#muti-app.html))上绑定天猫精灵账号(即淘宝账号).  

注意最后步骤, 否则天猫精灵无法找到设备:
> 在天猫精灵 APP 找到 "阿里智能" 技能, 手动进行 "尝试" 或 "设备同步"(后期会进行自动同步)  
> 即可在 "我家" 的设备列表中看到您的设备

完成以上步骤后，您可以通过天猫精灵音箱控制您的设备了. 您可以对天猫精灵说 "天猫精灵,开灯", "天猫精灵, 关灯", "天猫精灵, 把灯调成红色" 或者其他您希望设置的颜色, 设备即响应相应的命令.

##### 13.2 天猫精灵配网并控制设备
阿里云设备支持 `零配` 的配网方式.  
使设备进入配网状态, 对天猫精灵说 "天猫精灵,发现设备"  
天猫精灵回复 "正在为您扫描, 发现了智能灯, 现在连接吗"  
对天猫精灵说 "连接" 或者 "是的"  
天猫精灵回复 "好的, 设备连接中, 稍等一下下哦"  
设备收到天猫精灵发送的管理帧配网信息, 进行联网:  
![](_static/p19.png)

等待联网成功, 天猫精灵说 "智能设备联网成功, 现在用语音控制它试试", 这时您可以通过天猫精灵音箱控制您的设备了.

如果您之前通过云智能 APP 配网, 天猫精灵配网成功后, 云智能 APP 将不再显示设备. 如果继续通过云智能 APP 配网, APP 会配网失败, 显示 "设备添加失败, 设备已被管理员绑定, 请联系管理员解绑或将设备分享给您". 
> 在天猫精灵 APP 删除设备, 云智能 APP 再进行配网可以配置成功并显示设备.

#### 14.国际站设备开发
##### 14.1 创建国际站产品
在阿里云 [生活物联网平台-国际站](https://living-global.aliyun.com/#/) 创建产品, 参考[全球化服务文档](https://g.alicdn.com/aic/ilop-docs/2018.10.35/international.html)和[创建产品文档](https://living.aliyun.com/doc#readygo.html).  
同上述阿里云平台部署, 新增测试设备后获取设备的`三元组`, 参考 [量产说明](../../../config/mass_mfg/README.md) 文档烧录三元组到模组 NVS 分区.
##### 14.2 海外版 APP
手机从[阿里云官网](https://living.aliyun.com/doc#muti-app.html) 下载 `云智能` 公版 APP, 海外用户版.  
如果已下载过公版 APP(国内用户版), 退出登录, 重新注册.
注册时选择其他非“中国大陆”的地区，都认为是海外的 APP 账号，会默认连接国际站的服务器。  
目前在中国内地之外的国家和地区（包括港澳台地区）支持3个region：新加坡、美国和德国。中国内地之外的设备激活联网时，将统一连接到新加坡激活中心。在设备绑定时，平台将根据 App 用户所在区域，自动将设备切换到相应的region（示例如下）。

 - 示例一：App用户注册时选择的国家为美国，平台会将该用户待绑定的设备切换到美国的服务器。
 - 示例二：App用户注册时选择的国家为欧洲国家，平台会将该用户待绑定的设备切换到德国的服务器。
 - 示例三：App用户注册时选择的国家为东南亚国家，设备连接新加坡服务器，此时无需切换region。
##### 14.3 国际版固件设置
参考[国际站设备开发](https://help.aliyun.com/document_detail/138889.html).设备端固件需要修改以下配置来支持统一连接到新加坡激活中心。  
执行
```
 make menuconfig
```
选择 Componnet config -> iotkit embedded -> Aliyun linkkit mqtt config -> 取消选择 MQTT DIRECT.  
配置生效后会修改以下两个配置.
- 关闭 MQTT 直连功能, 打开 MQTT 预认证功能.
- 设置 linkkit_solo.c 中的 domain_type = IOTX_CLOUD_REGION_SINGAPORE;

##### 14.4 国际站设备运行
编译下载固件后, 使设备重新进入配网状态, APP 扫描 14.1 步骤中生成的配网二维码. 如果设备是第一次配网, 连网成功后连接新加坡服务器:  
![](_static/p-se.png)  
订阅 MQTT REDIRECT Topic:   
![](_static/p-sub.png)  
收到 MQTT REDIRECT Topic 后自动重启设备:  
![](_static/p-redirect.png)  
设备重启后, 由于本示例中 APP 注册时选择的地区是美国, 设备连接美国服务器:  
![](_static/p-us.png)  
手机 APP 端显示:  
![](_static/p23.jpg)  
点击"完成":  
![](_static/p22.jpg)  
稍等片刻后, 手机 APP 端显示产品状态, 此时可以通过手机控制设备:  
![](_static/p24.jpg)  