# STM32 主从舵机角度同步

本项目用于机器人战队电控组考核题“主从机舵机角度同步装置”。必做功能为
UART 角度同步，进阶功能为 CAN 总线角度同步。

当前硬件暂时无法插入调试，所以项目把角度映射、通信协议、CRC、超时保护等核心逻辑抽成
PC 可测试的 C 模块；STM32 目录提供 HAL/CubeMX 接入代码，后续拿到硬件后可直接移植调试。

## 功能

- 主机采集 `PA0/ADC1_IN0` 上 10K 电位器电压，并映射为 `0..180°`。
- UART 必做版使用 `USART1 PA9/PA10`，`9600 8N1`，每 `50ms` 发送一帧角度。
- CAN 进阶版使用 `PA12 CAN_TX / PA11 CAN_RX`，标准帧 ID `0x321`，DLC `4`。
- 从机使用 `TIM3_CH1/PA6` 输出 SG90 舵机 PWM：`0.5ms..2.5ms`，周期 `20ms`。
- 通信帧带 CRC8，错误帧丢弃；从机通信超时后回到 `90°` 安全角。
- ADC 使用滑动平均滤波，降低电位器噪声导致的舵机抖动。

## 目录结构

```text
core/       与平台无关的角度映射、协议、滤波和超时保护
firmware/   STM32 HAL/CubeMX 风格主机/从机应用代码
tests/      PC 端单元测试和长时间仿真
docs/       CubeMX 配置、接线和演示清单
stm32_mdk_project/ 已接入 Keil MDK 的 STM32F103C8T6 工程，可直接打开编译下载
```

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

## STM32 使用方式

若希望直接上板测试，优先打开：

```text
stm32_mdk_project/MDK-ARM/1.uvprojx
```

该工程已经接入核心逻辑、ADC、USART1、TIM3 PWM、CAN1、PC13 LED，并在本机通过 Keil
Arm Compiler 6 批量编译验证，生成过 `0 Error(s), 0 Warning(s)` 的固件。

1. 分别为主机和从机建立 STM32CubeMX 或 STM32CubeIDE 工程，芯片选择 `STM32F103C8T6`。
2. 按 [CubeMX 配置建议](docs/cubemx_setup.md) 配置 ADC、UART、CAN、TIM3 PWM 和 LED。
3. 将 `core/include`、`core/src` 加入 STM32 工程。
4. 复制 `firmware/app_entry.c`，并按目标功能复制对应应用文件。
5. 必做 UART 版复制：
   - `firmware/app_master_uart.c`
   - `firmware/app_slave_uart.c`
   - `firmware/common/board_config.h`
6. 进阶 CAN 版复制：
   - `firmware/app_master_can.c`
   - `firmware/app_slave_can.c`
   - `firmware/common/board_config.h`
7. 定义一组编译宏区分固件角色和通信方式：
   - 主机 UART：`BOARD_MASTER` + `LINK_UART`
   - 从机 UART：`BOARD_SLAVE` + `LINK_UART`
   - 主机 CAN：`BOARD_MASTER` + `LINK_CAN`
   - 从机 CAN：`BOARD_SLAVE` + `LINK_CAN`
8. 在 CubeMX 生成的 `main.c` 中，外设初始化完成后调用 `app_init()`，在主循环调用
   `app_task(HAL_GetTick())`。

当前命令行未发现 `arm-none-eabi-gcc`，因此仓库内暂不声称已完成 STM32 固件交叉编译。若安装
ARM GNU Toolchain 或使用 STM32CubeIDE，后续即可进行上板编译和烧录。

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
| 0 | `0xA5` |
| 1 | `0x5A` |
| 2 | 版本号 `0x01` |
| 3 | 序号 |
| 4 | 角度低字节，单位 `0.1°` |
| 5 | 角度高字节，单位 `0.1°` |
| 6 | CRC8 |

CAN payload 为 4 字节：`序号 + 角度低字节 + 角度高字节 + CRC8`。

## GitHub 上传

本地提交完成后，如果远端仓库已创建：

```powershell
git remote add origin https://github.com/<your-name>/<repo-name>.git
git push -u origin master
```

如果远端仓库已经存在并且本地也已设置 `origin`，直接执行：

```powershell
git push
```
