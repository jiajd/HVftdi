# HVftdi

Qt 编写的对 基于FT2232 的ISG250高压发生器与 FT2232HL开发板的测试程序

代码中有针对以下两个测试对象的描述信息的处理部分，如使用其他不同描述信息的FTDI芯片，需修改相应描述内容。

## 测试对象

### 1 ISG250高压发生器

### 2 FT2232HL-Board V3

【相关资料】
    
    1、该板原理图（地址：https://github.com/arm8686/FT2232HL-Board） 。
    
    2、芯片文档与驱动请到 FTDI 官网下载更新的版本（官网地址：http://www.ftdichip.com/）。
    
    3、《FT2232HL 用作 openOCD 简明手册 V0.1.pdf 》等文档
    
    链接：https://pan.baidu.com/s/1DYk8u500OdhF4Sqh5eX6Jg
    
    提取码：paxb
    
    关于 FT2232HL 的更多使用方式与功能，需要用户去了解 FTDI 公司的应用文档。

【简单使用步骤】

LED1 为上电指示灯。供电之后，常亮。

在安装其驱动 CDM21228_Setup 之后，FT2232HL 默认为 USB 转双串口。

使用 PC 端串口调试软件可以便捷地测试这两个串口。

只需使用两根杜邦线进行串口的交叉连接：

    ADBUS0 (TXD)  接 BDBUS1 (RXD)
    
    ADBUS1 (RXD)  接 BDBUS0 (TXD)
