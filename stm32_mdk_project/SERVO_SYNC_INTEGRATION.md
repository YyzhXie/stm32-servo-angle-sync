# Servo Sync 集成说明

本 STM32 工程已经接入电控考核项目代码，默认编译为：

```c
BOARD_MASTER + LINK_UART
```

也就是“主机通过 PA0 采集电位器，通过 USART1/PA9 发送角度”的必做版本。

## 当前验证状态

- 已修复 µVision 曾提示的 `Cannot read project file ... 1.uvprojx` 工程文件读取问题。
- 已用 Keil MDK / Arm Compiler 6 批量编译通过，结果为 `0 Error(s), 0 Warning(s)`。
- 由于当前没有办法立刻插入硬件，尚未完成真实电位器、舵机和 CAN 总线闭环测试。
- 上板时建议先验证 UART 必做链路，再验证 CAN 进阶链路。

## 切换主机/从机与 UART/CAN

修改 `Core/Inc/app_config.h`：

| 目标 | 宏配置 |
| --- | --- |
| 主机 UART 必做版 | `BOARD_MASTER` + `LINK_UART` |
| 从机 UART 必做版 | `BOARD_SLAVE` + `LINK_UART` |
| 主机 CAN 进阶版 | `BOARD_MASTER` + `LINK_CAN` |
| 从机 CAN 进阶版 | `BOARD_SLAVE` + `LINK_CAN` |

每次给不同板子烧录前，只需要改这里并重新编译下载。

## 已接入外设

| 功能 | 引脚/外设 |
| --- | --- |
| 电位器 ADC | `PA0 / ADC1_IN0` |
| UART 通信 | `USART1 PA9(TX), PA10(RX), 9600 8N1` |
| 舵机 PWM | `TIM3_CH1 / PA6`, 50Hz, 0.5ms-2.5ms |
| CAN 进阶 | `CAN1 PA12(TX), PA11(RX)`, 1Mbps, 标准帧 ID `0x321` |
| 状态 LED | `PC13`, 默认低电平点亮 |

## 测试顺序

1. 先用默认 `BOARD_MASTER + LINK_UART` 烧主机，电位器中间脚接 `PA0`。
2. 改为 `BOARD_SLAVE + LINK_UART` 烧从机，舵机信号接 `PA6`。
3. 主机 `PA9` 接从机 `PA10`，两块板和舵机电源必须共地。
4. 旋转电位器，观察舵机同步转动。
5. 若测试 CAN，再分别切换为 `LINK_CAN` 的主机/从机版本，接 TJA1050 的 CANH/CANL。

## CubeMX 注意事项

当前外设初始化和 Keil 工程文件已经手工补齐并通过 ArmClang 语法编译检查。
但 `1.ioc` 原本是空外设工程，没有同步完整外设配置；如果直接用 CubeMX 重新生成，可能覆盖这些手工集成内容。

回去测试建议直接打开 `MDK-ARM/1.uvprojx` 编译下载。若必须重新生成，请先在 CubeMX 中配置同样的 ADC1、USART1、TIM3_CH1、CAN1、PC13 后再生成。
