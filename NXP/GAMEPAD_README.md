# 游戏手柄按键功能说明

## 核心架构

按照 **职责分离** 原则,代码分为三层:

```
Hardware Layer    →  key_app.c/h       (4x4矩阵键盘扫描)
                     ↓
Protocol Layer    →  usb_app.c/h       (USB HID通信)
                     ↓
Application Layer →  gamepad_app.c/h   (按键映射+业务逻辑)
```

## 文件说明

### 1. applications/key_app.c/h
**硬件抽象层** - 负责矩阵键盘扫描

- `key_read()`: 扫描4x4矩阵键盘,返回0-15(按键索引)或0xFF(无按键)
- 无业务逻辑,纯硬件操作

### 2. applications/usb_app.c/h
**协议层** - USB HID游戏手柄协议实现

- `hid_gamepad_init()`: 初始化USB HID设备
- `hid_gamepad_send_report()`: 发送手柄报告到PC
- `hid_gamepad_get_report()`: 获取报告缓冲区
- 支持16按钮+4轴摇杆+2扳机+方向键

### 3. applications/gamepad_app.c/h
**应用层** - 按键到手柄的映射逻辑 **(本次新增)**

- 定期调用 `key_read()` 扫描按键
- 将按键索引(0-15)映射到手柄按钮(bit0-bit15)
- 自动调用 `hid_gamepad_send_report()` 发送USB报告

**核心映射逻辑**(3行代码):
```c
uint8_t key = key_read();
report->buttons = (key != 0xFF) ? (1 << key) : 0;  // 位映射
hid_gamepad_send_report(0, report);                 // 发送
```

### 4. board/ports/cherryusb/cherryusb.c
**USB硬件初始化** - 在系统启动时自动初始化USB HID

## 按键映射表

4x4矩阵键盘 → USB手柄按钮(一一对应):

```
     C1    C2    C3    C4
R1  [0]   [1]   [2]   [3]    →  Button0-3  (A/B/X/Y)
R2  [4]   [5]   [6]   [7]    →  Button4-7  (LB/RB/Back/Start)
R3  [8]   [9]   [10]  [11]   →  Button8-11 (LS/RS/...)
R4  [12]  [13]  [14]  [15]   →  Button12-15
```

可在 `gamepad_app.h` 中自定义别名(例如KEY_A=0对应按键0)。

## 编译与运行

### 1. 确保USB已启用
检查 `.config` 或 `rtconfig.h`:
```c
#define RT_CHERRYUSB_DEVICE_TEMPLATE_HID_KEYBOARD
```

### 2. 编译
```bash
scons -c    # 清理
scons       # 编译
```

### 3. 烧录运行
烧录 `rtthread.bin` 到开发板,USB会自动识别为游戏手柄。

### 4. 测试

- **按键测试**: 按下任意键盘按键,对应手柄按钮被触发
- **USB测试**: 在FinSH终端输入 `hid_example`,运行摇杆旋转测试
- **Windows测试**: 打开"设置→游戏→游戏控制器"查看按键状态

## 调试输出

系统会在串口打印:
```
[USB] Initializing HID Gamepad...
[USB] Device Configured - Gamepad Ready!
[GAMEPAD] Thread started
[GAMEPAD] Button 5 pressed (bit 5)   ← 按键5按下
[GAMEPAD] All buttons released        ← 按键释放
```

## 优化建议

### 消除重复扫描(可选)

当前 `main.c` 中有一个独立的按键扫描线程用于串口打印调试。如果不需要串口调试,可以删除 `main.c` 中的:
- `thread_key()` 函数
- `thread_print()` 函数
- `keySem` 信号量

只保留 `gamepad_app.c` 中的扫描逻辑,避免两个线程同时扫描按键。

**删除后的 main.c**:
```c
int main(void)
{
    rt_kprintf("System Start\r\n");
    return 0;
}
```

## 代码风格

遵循 **"Good Taste"** 原则:
- ✅ 消除特殊情况: 使用位运算 `1 << key` 替代16个if判断
- ✅ 数据驱动: 按键索引天然对应按钮位
- ✅ 零复杂度: 核心逻辑3行代码
- ✅ 职责分离: 硬件/协议/应用三层清晰

## 常见问题

**Q: PC识别不到手柄?**
A: 检查USB线连接,查看串口是否输出 `[USB] Device Configured`

**Q: 按键无响应?**
A: 检查 `gamepad_app.c` 线程是否启动,查看串口日志

**Q: 想改按键映射?**
A: 修改 `gamepad_app.h` 中的KEY_A等宏定义

**Q: 需要摇杆功能?**
A: 在 `gamepad_app.c` 中修改 `report->left_x/left_y` 等字段
