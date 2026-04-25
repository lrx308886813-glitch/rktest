# elder-care-terminal

一个基于 C++17/20 + CMake 的居家养老终端骨架工程（无 Python 运行时依赖）。

## 目录概览
- `include/` / `src/`：core/services/hardware/web 分层实现，头源对应。
- `web/`：静态页面和 JS/CSS。
- `config/`：`device.ini`、`reminders.ini`、`contacts.ini` 默认配置。
- `data/`：`*.db.json` 与 `event_log.txt` 持久化目录（不存在时自动创建）。
- `tools/factory_test.cpp`：产测工具。
- `deploy/systemd/elder-care-terminal.service`：systemd 服务文件。

## 编译
```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## 运行
在仓库根目录执行：
```bash
./build/elder_care_terminal
```

## 访问路径
- `GET /`：主页
- `GET /admin`：管理页
- `GET /api/health`
- `GET /api/messages`
- `GET /api/contacts`
- `GET /api/reminders`
- 其他 `GET /api/*` 返回 JSON 错误对象

## RK3566 部署说明
当前仓库默认使用 mock 硬件，`Rk3566HardwarePlaceholder` 预留真实实现接口：
- 音频：ALSA/aplay（TODO）
- 摄像头：V4L2/OpenCV（TODO）
- 通信：4G/SIP/WebRTC（TODO）
- GPIO：真实 gpiochip 访问（TODO）

请在 `config/device.ini` 中填充真实设备路径/GPIO，不要硬编码到源码。

## systemd 安装
```bash
sudo cp elder-care-terminal/deploy/systemd/elder-care-terminal.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable elder-care-terminal
sudo systemctl start elder-care-terminal
```

## 真实功能与 mock/TODO 边界
已实现：
- 最小 HTTP 服务与静态托管。
- 每 10 秒提醒轮询、触发 `REMINDER_ALERT`、播放告警、3 分钟未确认写日志。
- `messages/contacts/reminders` JSON 持久化。
- 事件日志与状态切换日志。

mock/TODO：
- 真正的音视频/GPIO/通信栈。
- 完整业务 API（目前提供最小可运行接口）。
