# STM32 主从舵机角度同步

本项目用于机器人战队电控组考核题“主从机舵机角度同步装置”。必做功能为 UART 角度同步，进阶功能为 CAN 总线角度同步。

项目语言为 C11。为了应对当前暂时无法插入真实硬件调试的情况，角度映射、通信协议、CRC、滤波、超时保护等核心逻辑被抽成了可在 PC 上编译测试的模块；STM32 侧提供 HAL/CubeMX 风格代码，并已经集成到一个 Keil MDK 工程中，后续可直接上板验证。

## 当前交付状态

- 已完成 UART 必做版：主机采集电位器角度，从机接收角度并输出 SG90 PWM。
- 已完成 CAN 进阶版代码：标准帧 ID `0x321`，payload 内含序号、角度和 CRC8。
- 已完成 PC 离线测试：核心算法和通信协议可用 CMake/Ninja/MinGW 编译测试。
- 已完成 Keil MDK 集成：`stm32_mdk_project/MDK-ARM/1.uvprojx` 可直接打开。
- 已在本机验证 Keil 编译：Arm Compiler 6 构建结果为 `0 Error(s), 0 Warning(s)`。
- 尚未完成真实硬件闭环测试：原因是当前手头硬件暂时无法插入环境调试，后续按 `docs/demo_checklist.md` 上板验收。

## 功能设计

- 主机采集 `PA0/ADC1_IN0` 上的 10K 电位器电压，并映射为 `0..180°`。
- 内部角度统一使用 `0.1°` 单位保存，例如 `900` 表示 `90.0°`。
- UART 必做版使用 `USART1 PA9/PA10`，`9600 8N1`，每 `50ms` 发送一帧角度。
- CAN 进阶版使用 `PA12 CAN_TX / PA11 CAN_RX`，标准帧 ID `0x321`，DLC `4`。
- 从机使用 `TIM3_CH1/PA6` 输出 SG90 舵机 PWM：`0.5ms..2.5ms`，周期 `20ms`。
- 通信帧带 CRC8，错误帧丢弃；从机通信超时后回到 `90°` 安全角。
- ADC 使用滑动平均滤波，降低电位器噪声导致的舵机抖动。
- `PC13` 作为状态 LED：正常通信时闪烁或熄灭，错误/超时时进入错误指示。

## 目录结构

```text
core/                 与平台无关的角度映射、协议、滤波和超时保护
firmware/             STM32 HAL/CubeMX 风格主机/从机应用代码
tests/                PC 端单元测试和长时间仿真
docs/                 CubeMX 配置、接线、演示和 GitHub 流程说明
stm32_mdk_project/    已接入 Keil MDK 的 STM32F103C8T6 工程
```

## 推荐打开方式

若只看算法或跑 PC 测试，打开仓库根目录。

若要回去直接上板测试，优先打开：

```text
stm32_mdk_project/MDK-ARM/1.uvprojx
```

该工程已经接入核心逻辑、ADC、USART1、TIM3 PWM、CAN1、PC13 LED。默认配置为：

```c
BOARD_MASTER + LINK_UART
```

也就是“主机采集 PA0 电位器，并通过 USART1/PA9 发送角度”。烧录从机或切换 CAN 时，修改：

```text
stm32_mdk_project/Core/Inc/app_config.h
```

| 目标 | 宏配置 |
| --- | --- |
| 主机 UART 必做版 | `BOARD_MASTER` + `LINK_UART` |
| 从机 UART 必做版 | `BOARD_SLAVE` + `LINK_UART` |
| 主机 CAN 进阶版 | `BOARD_MASTER` + `LINK_CAN` |
| 从机 CAN 进阶版 | `BOARD_SLAVE` + `LINK_CAN` |

## PC 离线构建与测试

已确认本机 CLion 自带工具可用：

- CMake `4.2.2`
- Ninja `1.13.2`
- MinGW GCC `13.1.0`

在 PowerShell 中可执行：

```powershell
$env:PATH="D:\CLion 2026.1.2\bin\mingw\bin;$env:PATH"
& "D:\CLion 2026.1.2\bin\cmake\win\x64\bin\cmake.exe" -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM="D:\CLion 2026.1.2\bin\ninja\win\x64\ninja.exe" -DCMAKE_C_COMPILER="D:\CLion 2026.1.2\bin\mingw\bin\gcc.exe"
& "D:\CLion 2026.1.2\bin\cmake\win\x64\bin\cmake.exe" --build build
& "D:\CLion 2026.1.2\bin\cmake\win\x64\bin\ctest.exe" --test-dir build --output-on-failure
```

也可以直接用 CLion 打开本目录，选择 bundled MinGW/Ninja 工具链后运行测试目标。

## Keil MDK 构建

在本机可用如下命令批量编译集成工程：

```powershell
& "D:\Code\MDK-ARM\UV4\UV4.exe" -b "stm32_mdk_project\MDK-ARM\1.uvprojx" -t "1" -o "stm32_mdk_project\MDK-ARM\build.log"
```

已经验证过的结果：

```text
"1\1.axf" - 0 Error(s), 0 Warning(s).
```

说明：此前遇到过 µVision 提示 `Cannot read project file ... 1.uvprojx`，后续已修复工程文件格式并重新验证通过。

## 硬件接线

详细表格见 [接线说明](docs/wiring.md)。

关键点：

- UART 必做同步：主机 `PA9 TX` 接从机 `PA10 RX`，两板共地。
- 电位器中间脚接主机 `PA0`，两端接 `3.3V` 和 `GND`。
- SG90 信号线接从机 `PA6/TIM3_CH1`，舵机 5V 供电需与 STM32 共地。
- CAN 进阶同步：两块 TJA1050 的 CANH/CANL 对接，STM32 侧用 `PA12/PA11`。

## 协议说明

UART 帧长 7 字节：

| 字节 | 含义 |
| --- | --- |
| 0 | 帧头 `0xA5` |
| 1 | 帧头 `0x5A` |
| 2 | 版本号 `0x01` |
| 3 | 序号 |
| 4 | 角度低字节，单位 `0.1°` |
| 5 | 角度高字节，单位 `0.1°` |
| 6 | CRC8 |

CAN payload 为 4 字节：`序号 + 角度低字节 + 角度高字节 + CRC8`。

CRC8 使用多项式 `0x07`，接收端校验失败则丢弃该帧，不更新舵机角度。

## 上板验收顺序

推荐按 [演示与调试清单](docs/demo_checklist.md) 执行：

1. 先烧主机 UART 版，确认 PA0 电位器输入能转换为连续角度。
2. 再烧从机 UART 版，确认 PA6 PWM 周期为 `20ms`，脉宽在 `0.5ms..2.5ms`。
3. 连接主机 `PA9` 到从机 `PA10`，两板共地，观察舵机同步。
4. 断开通信，确认从机超时回到 `90°` 安全角。
5. 再切换到 CAN 版，接入 TJA1050 模块验证加分功能。

## GitHub 上传

当前仓库已推送至：

```text
https://github.com/YyzhXie/stm32-servo-angle-sync
```

GitHub CLI 登录后的创建、提交和推送流程见 [GitHub 工作流](docs/github_workflow.md)。
