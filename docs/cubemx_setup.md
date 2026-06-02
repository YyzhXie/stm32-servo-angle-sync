# CubeMX / STM32CubeIDE 配置建议

目标芯片为 `STM32F103C8T6`。本目录代码按 HAL 工程写法组织，适合复制到 CubeMX
生成的工程中使用。

如果只是回去测试现成工程，优先使用 `stm32_mdk_project/MDK-ARM/1.uvprojx`。该工程已经手工接入
ADC、USART1、TIM3 PWM、CAN1 和 PC13 LED，并通过 Keil 编译。只有在需要重新生成 CubeMX 工程时，
才按本页重新配置外设。

## 主机 UART 必做版

| 外设 | 配置 |
| --- | --- |
| `ADC1` | `IN0/PA0`，单通道，12 位，软件触发 |
| `USART1` | `PA9 TX`，`PA10 RX`，`9600 8N1` |
| `GPIO` | `PC13` 输出，作为状态 LED |

主循环调用：

```c
app_master_uart_init();
while (1) {
    app_master_uart_task(HAL_GetTick());
}
```

## 从机 UART 必做版

| 外设 | 配置 |
| --- | --- |
| `USART1` | `PA9 TX`，`PA10 RX`，`9600 8N1`，打开中断 |
| `TIM3_CH1` | `PA6`，PWM 输出，50Hz |
| `GPIO` | `PC13` 输出，作为状态 LED |

建议 TIM3 设置：计数频率 `1MHz`，自动重装值 `19999`，PWM 周期即 `20ms`。
代码中直接写 `CCR = 500..2500`，对应 `0.5ms..2.5ms`。

## CAN 进阶版

| 外设 | 配置 |
| --- | --- |
| `CAN` | `PA12 CAN_TX`，`PA11 CAN_RX`，1Mbps，标准帧 |
| `CAN ID` | `0x321` |
| `TJA1050` | 两端模块共地，CANH 对 CANH，CANL 对 CANL |

CAN 位时序需根据 APB1 时钟调整。若 APB1 为 36MHz，可先尝试：

| 参数 | 值 |
| --- | --- |
| Prescaler | 4 |
| TimeSeg1 | 6TQ |
| TimeSeg2 | 2TQ |
| SJW | 1TQ |

如果现场 CAN 不通，优先检查：两板共地、CANH/CANL 是否接反、TJA1050 供电、波特率是否一致。
