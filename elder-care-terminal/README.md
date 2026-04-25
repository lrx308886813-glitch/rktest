# elder-care-terminal

基于 RK3566 Linux 的老人陪伴提醒终端第一版用户态应用。当前版本使用 C++17 + CMake，内置 HTTP 服务和简单 Web 页面，默认 mock 模式可在普通 Ubuntu/Linux 环境运行。

## 编译

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

## 运行

```bash
cd build
./elder-care-terminal
```

程序会自动创建 `data/` 目录，用本地 JSON/文本文件保存提醒、留言、联系人和事件日志。

## 访问

- 老人端页面：http://127.0.0.1:8080
- 管理后台：http://127.0.0.1:8080/admin
- 健康检查：http://127.0.0.1:8080/api/health

## RK3566 部署

1. 在 RK3566 Linux SDK 或目标板上安装 CMake 和 C++17 编译器。
2. 编译工程并把程序、`config/`、`web/`、`deploy/`、`docs/` 复制到 `/opt/elder-care-terminal`。
3. 根据真机设备修改 `config/device.ini`，不要在代码里写死摄像头、音频或 GPIO 参数。
4. 安装服务：

```bash
sudo cp deploy/systemd/elder-care-terminal.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable elder-care-terminal
sudo systemctl start elder-care-terminal
```

## 已实现

- C++ 后端 HTTP 服务。
- 老人端大字体页面和管理后台。
- 提醒、留言、联系人、设备状态和日志 API。
- 本地文件存储，不依赖数据库。
- 提醒后台线程每 10 秒检查，到时播放 mock 提醒并进入 `REMINDER_ALERT`。
- 状态机日志。
- mock 音频、mock 摄像头、mock GPIO。
- 工厂测试工具 `factory_test`。

## Mock/TODO

- RK3566 摄像头路径、音频设备和 GPIO 编号仍在 `config/device.ini` 中标记为 `TODO`。
- `Rk3566HardwarePlaceholder` 预留 V4L2/OpenCV、ALSA/aplay、GPIO、4G/SIP/WebRTC 扩展点。
- 语音留言未实现，当前只支持文字留言。
- 一键呼叫当前只记录日志，不进行真实拨号。
- Kiosk 模式由系统层配置浏览器自启动，本工程只提供页面和服务。
