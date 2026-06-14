# 硬件连接图

正式配置：`BOARD_MASTER + LINK_UART` / `BOARD_SLAVE + LINK_UART`。

```mermaid
flowchart LR
    POT["10K 电位器<br/>左脚 GND<br/>中间脚 PA0<br/>右脚 3.3V"]
    MASTER["主板 STM32F103C8T6<br/>BOARD_MASTER + LINK_UART"]
    SLAVE["从板 STM32F103C8T6<br/>BOARD_SLAVE + LINK_UART"]
    SERVO_PWR["外部 5V 电源模块<br/>5V / GND"]
    SERVO["SG90 舵机<br/>红线 5V<br/>棕/黑线 GND<br/>黄/橙线信号"]
    STLINK["ST-Link 烧录器<br/>SWDIO / SWCLK / GND / 3.3V"]

    POT -->|"PA0 / ADC1_IN0"| MASTER
    MASTER -->|"PA9 TX -> PA10 RX<br/>9600 8N1"| SLAVE
    MASTER ---|"GND 共地"| SLAVE
    SLAVE -->|"PA6 / TIM3_CH1<br/>50Hz PWM"| SERVO
    SERVO_PWR -->|"5V"| SERVO
    SERVO_PWR ---|"GND 共地"| SLAVE
    STLINK -.->|"烧录时连接目标板 PA13/PA14"| MASTER
    STLINK -.->|"烧录时连接目标板 PA13/PA14"| SLAVE
```

## 接线清单

| 模块 | 引脚 | 连接到 | 说明 |
| --- | --- | --- | --- |
| 电位器 | 左脚 | 主板 `GND` | 方向反时交换左右脚 |
| 电位器 | 中间脚 | 主板 `PA0` | ADC 输入 |
| 电位器 | 右脚 | 主板 `3.3V` | 不要接 5V 到 PA0 |
| 主板 | `PA9 TX` | 从板 `PA10 RX` | UART |
| 主板 | `GND` | 从板 `GND` | 共地 |
| 从板 | `PA6 / TIM3_CH1` | 舵机信号线 | 50Hz PWM |
| 舵机 | 红线 | 外部 `5V` | 独立供电 |
| 舵机 | 棕/黑线 | 外部电源 `GND` | 与从板共地 |
| ST-Link | `SWDIO/SWCLK` | 目标板 `PA13/PA14` | 烧录 |
| ST-Link | `GND/3.3V` | 目标板 `GND/3V3` | `BOOT0 = 0` |

## 关键提示

- 从板 PC13 常亮：未收到有效 UART 帧。
- 舵机不动：先查 `5V` 和共地。
- 烧录失败：只保留 `3.3V/GND/SWDIO/SWCLK/NRST`。
