# RK3566 Porting

第一版不涉及 Kernel、BSP、Device Tree，只做 Linux 用户态应用。

真机适配建议顺序：

1. 确认屏幕分辨率和 kiosk 浏览器启动方式。
2. 确认摄像头节点，补 `Rk3566HardwarePlaceholder` 中的 V4L2/OpenCV 实现。
3. 确认音频输出设备，补 ALSA 或 `aplay` 调用。
4. 确认按键 GPIO 所在 gpiochip 和 line，补 libgpiod 或 sysfs 兼容层。
5. 根据产品通信方案选择 4G、SIP 或 WebRTC，再实现真实呼叫。
6. 增加系统级看门狗、日志轮转和远程升级。

`config/device.ini` 是真机差异的入口；新增字段时优先扩展配置，不修改业务层常量。
