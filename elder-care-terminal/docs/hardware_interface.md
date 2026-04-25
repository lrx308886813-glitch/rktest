# Hardware Interface

当前硬件接口只定义抽象层，不绑定真实 RK3566 设备路径。

- `IAudio`：`playAlert()`、`playMessage()`、`stop()`。
- `ICamera`：`detectPresence()`。
- `IGpio`：`readButton()`。

默认实现：

- `MockAudio` 只写日志。
- `MockCamera` 返回配置值或简单随机结果。
- `MockGpio` 默认返回未按下。

真机参数全部放在 `config/device.ini`：

- `camera_device=TODO`
- `audio_device=TODO`
- `gpio_confirm_button=TODO`
- `gpio_call_button=TODO`

不要在业务代码里写死设备节点、声卡编号或 GPIO line。
