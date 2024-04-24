
[tag download]:https://github.com/Jieli-Tech/fw-AW31N_BLE_SDK/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/fw-AW31N_BLE_SDK?style=plastic&logo=bluetooth&labelColor=ffffff&color=informational&label=Tag&logoColor=blue

# fw-AW31N_BLE_SDK   [![tag][tag_badgen]][tag download]

中文 | [EN](./README-en.md)

AW31N系列通用蓝牙SDK 固件程序

本仓库包含SDK release 版本代码，线下线上支持同步发布

本工程提供的例子，需要结合对应命名规则的库文件(lib.a) 和对应的子仓库进行编译.

快速开始
------------

欢迎使用杰理开源项目，在开始进入项目之前，请详细阅读SDK 介绍文档，
从而获得对杰理系列芯片和SDK 的大概认识，并且可以通过快速开始介绍来进行开发.


工具链
------------

关于如何获取`杰理工具链` 和 如何进行环境搭建，请阅读以下内容：

* 编译工具 ：请安装杰理编译工具来搭建起编译环境, [下载链接](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html) 

* USB 升级工具 : 在开发完成后，需要使用杰理烧写工具将对应的`hex`文件烧录到目标板，进行开发调试, 关于如何获取工具请进入申请 [链接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.5.504d246bXKwyeH&id=620295020803) 并详细阅读对应的[文档](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/index.html)

介绍文档
------------

* 芯片简介 : [SoC 数据手册扼要](https://doc.zh-jieli.com/vue/#/docs/aw31n), [下载链接](./doc/AW31N_规格书)

* 应用设计原理图参考 : [下载链接](./doc/AW31N_原理图)

* SDK 版本信息 : [SDK 历史版本](https://doc.zh-jieli.com/AW31/zh-cn/master/other/version/index.html)

* SDK 介绍文档 : [SDK 快速开始简介](https://doc.zh-jieli.com/AW31/zh-cn/master/getting_started/preparation/index.html)

* SDK 结构文档 : [SDK 模块结构](https://doc.zh-jieli.com/AW31/zh-cn/master/getting_started/sdk_app_develop/sdk_catalog.html)

编译工程
-------------
请选择以下一个工程进行编译，下列目录包含了便于开发的工程文件：

* 蓝牙应用 : [TRANSFER](./apps/demo/transfer/), 适用领域：透传, 数传, 扫描设备, 广播设备.
* 蓝牙应用 : [HID](./apps/demo/hid/), 适用领域：遥控器, 自拍器, 键盘, 鼠标.翻页器
* [文档链接](https://doc.zh-jieli.com/AW31/zh-cn/master/module_demo/index.html)

已发布版本详见 标签(Tags)。

即将发布：

* 蓝牙应用 ：`Dongle + Mouse（1K）`


SDK 同时支持Codeblock 和 Make 编译环境，请确保编译前已经搭建好编译环境，

* Codeblock 编译 : 进入对应的工程目录并找到后缀为 `.cbp` 的文件, 双击打开便可进行编译.

* Makefile 编译 : 双击`tools/make_prompt.bat`，输入 `make target`（具体`target`的名字，参考`Makefile`开头的注释）

  `在编译下载代码前，请确保USB 升级工具正确连接并且进入编程模式`

蓝牙官方认证
-------------

低功耗蓝牙Link Layer 层和Host 协议栈均支持5.4版本实现

* Core v5.4 [QDID 223418](https://launchstudio.bluetooth.com/ListingDetails/193923)


硬件环境
-------------

* 开发评估板 ：开发板申请入口：敬请期待

* 生产烧写工具 : 为量产和裸片烧写而设计,工具需联系代理商获取，使用前请仔细阅读相关 [文档](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html)

* 无线测试盒 : 为空中升级/射频标定/快速产品测试而设计, 申请入口 [链接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.10.504d246bXKwyeH&id=620942507511), 阅读[文档](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/testbox_1tuo2/index.html) 获取更多详细信息.


社区
--------------

* 技术交流群，钉钉群 ID: 90400000565

* 常见问题集合[链接](https://doc.zh-jieli.com/AW31/zh-cn/master/other/faq/index.html)

免责声明
------------

- AW31N_BLE_SDK 支持AW31N系列芯片开发.
- AW31N 系列芯片支持了通用蓝牙常见应用，可以作为开发，评估，样品，甚至量产使用，对应SDK 版本见tag 和 release
