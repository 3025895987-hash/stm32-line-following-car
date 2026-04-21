# STM32 Line Following Car

基于 STM32F103C8 的循迹小车控制工程，包含电机驱动、舵机控制、PID 调节、串口通信、超声波测距与姿态模块相关代码，适合作为嵌入式课程设计、竞赛练习与小车控制项目的参考工程。

## 项目简介

本项目是一个基于 STM32 的智能循迹小车下位机工程，使用 Keil 工程进行开发与管理。  
工程中包含了较完整的小车控制基础模块，能够用于循迹控制、运动控制与外设联调。

## 主要功能

- 循迹控制
- 电机 PWM 驱动
- 舵机转向控制
- PID 参数调节
- 串口通信
- 超声波测距
- OLED 显示
- 姿态模块数据接入

## 硬件平台

- 主控芯片：STM32F103C8
- 开发环境：Keil MDK
- 电机驱动模块：已在 `Hardware/` 中提供相关控制代码
- 舵机模块：已在 `Hardware/Servo.*` 中提供相关控制代码
- 超声波模块：HC-SR04
- 姿态模块：JY61P
- 显示模块：OLED

## 项目结构

```text
.
├── User/              # 主函数与应用层逻辑
├── Hardware/          # 电机、舵机、串口、超声波、OLED 等底层驱动
├── System/            # PID、定时器、串口中断等系统级功能
├── Start/             # 启动文件
├── Library/           # STM32 标准外设库
├── docs/              # 补充说明文档
└── Project.uvprojx    # Keil 工程文件
如何打开工程
使用 Keil MDK 打开 Project.uvprojx
选择对应的 Target
检查芯片型号与下载器配置
编译工程并下载到开发板
开发说明

本仓库保留了适合继续开发和阅读的核心源码与工程文件。
编译生成文件、临时文件以及本机相关配置文件未保留，Keil 可在本地重新生成。

代码说明

工程中的主要模块包括：

User/main.c：程序入口与整体流程控制
Hardware/Motor.*：电机驱动
Hardware/Servo.*：舵机控制
Hardware/Serial.*：串口发送与接收
Hardware/Hcsr04.*：超声波测距
Hardware/OLED.*：OLED 显示
System/PID.*：PID 控制算法
System/USART3.*：串口 3 相关功能
Hardware/Drive.*：小车运动控制封装
文档
docs/serial_protocol.md：串口通信相关说明
docs/hardware_notes.md：硬件连接与模块说明
使用说明
根据自己的硬件连接检查各外设引脚配置
根据实际小车结构调整 PID 参数
根据传感器安装位置与底盘结构调整控制逻辑
下载到开发板后进行联调与测试
适用场景
STM32 嵌入式学习
智能小车项目开发
课程设计
电子设计竞赛/机器人竞赛基础工程参考
License

This project is licensed under the MIT License.
