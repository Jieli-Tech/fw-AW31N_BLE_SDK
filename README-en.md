
[tag download]:https://github.com/Jieli-Tech/fw-AW31N_BLE_SDK/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/fw-AW31N_BLE_SDK?style=plastic&logo=bluetooth&labelColor=ffffff&color=informational&label=Tag&logoColor=blue

# fw-AW31N_BLE_SDK   [![tag][tag_badgen]][tag download]

[Chinese](./README.md) | EN

General Bluetooth SDK Firmware for the AW31N Series

This repository contains the SDK release version of the code, with both online and offline support synchronized.

Examples provided in this project must be compiled in combination with the corresponding named library files (lib.a) and the corresponding sub-repositories.

Quick Start
------------

Welcome to the Jieli open-source project. Before you start the project, please read the SDK introduction document in detail,
to get a general understanding of the Jieli series chips and SDK, and you can start development through the quick start guide.


Toolchain
------------

To learn how to obtain the 'Jieli Toolchain' and how to set up the environment, please read the following:

* Compilation Tools: Please install the Jieli compilation tools to set up the compilation environment, [Download Link](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html)  

* USB Upgrade Tool: After development, you need to use the Jieli burning tool to program the corresponding `hex` file into the target board for development and debugging. To obtain the tool, please apply [Link](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.5.504d246bXKwyeH&id=620295020803) and read the corresponding [Documentation](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/index.html) 

Introduction Documents
------------

* Chip Introduction: [SoC Overview](https://doc.zh-jieli.com/vue/#/docs/aw31n), [Download Link](./doc/AW31N_规格书)

* Application design schematic：[Download Link](./doc/AW31N_原理图)

* SDK Version Information: [SDK Version History](https://doc.zh-jieli.com/AW31/zh-cn/master/other/version/index.html)

* SDK Quick Start Guide: [SDK Quick Start Introduction](https://doc.zh-jieli.com/AW31/zh-cn/master/getting_started/preparation/index.html)

* SDK Structure Documentation: [SDK Module Structure](https://doc.zh-jieli.com/AW31/zh-cn/master/getting_started/sdk_app_develop/sdk_catalog.html)

Compile Projects
-------------

Please choose one of the following projects to compile. The directories below contain project files that facilitate development:

* Bluetooth Applications: [TRANSFER](./apps/demo/transfer), applicable for: Transparent Transmission, Data Transmission, Scanning Device, Broadcasting Device.
* Bluetooth Applications: [HID](./apps/demo/hid), applicable for: Remote Control, Self-Timer, Keyboard, Mouse, Page Turner.
* [Documentation Link](https://doc.zh-jieli.com/AW31/zh-cn/master/module_demo/index.html)

Please refer to the Tags for the released versions.

Coming Soon:

* Bluetooth Application: `Dongle + Mouse（1K）`


The SDK support Codeblock & Make to build to project,make sure you already setup the enviroment

* Codeblock build : enter the project directory and find the `.cbp`,double click and build.

* Makefile build : double click the `tools/make_prompt.bat` and excute `make target`(see `makfile`)

  `Before compiling and downloading the code, please ensure that the USB upgrade tool is correctly connected and in programming mode`

Bluetooth Official Certification
-------------

Low Power Bluetooth Link Layer and Host Protocol Stack support the implementation of version 5.4

* Core v5.4 [QDID 223418](https://launchstudio.bluetooth.com/ListingDetails/193923) 


Hardware Environment
-------------

* Development Evaluation Board: Entrance for development board application [Link](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html)

* Production Burning Tools: Designed for mass production and bare chip programming, Contact agent and carefully read the related [Documentation](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) 

* Wireless Test Box: Designed for Over-The-Air updating/RF calibration/quick product testing, application entrance [Link](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.10.504d246bXKwyeH&id=620942507511), read the [Documentation](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/testbox_1tuo2/index.html) for more details.


Community
--------------

* Technical exchange group, DingTalk group ID: 90400000565

* FAQ collection [Link](https://doc.zh-jieli.com/AW31/zh-cn/master/other/faq/index.html)

Disclaimer
------------

- AW31N_BLE_SDK supports the development of the AW31N series chips.
- The AW31N series chips support common Bluetooth applications and can be used for development, evaluation, sampling, and even mass production. For corresponding SDK versions, see tags and releases.
