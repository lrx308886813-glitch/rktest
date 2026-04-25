# Architecture

第一版是单进程 Linux 用户态应用：

- `App` 负责装配配置、存储、硬件抽象、业务服务和 HTTP 服务。
- `HttpServer` 提供静态页面和 REST API。
- `Storage` 使用 `data/` 下的 JSON/文本文件保存本地数据。
- `ReminderService` 启动后台线程，每 10 秒检查一次提醒。
- `AudioService`、`CameraService`、`CallService` 通过抽象层隔离真机适配。

## 状态机

状态：

- `BOOT`
- `IDLE`
- `REMINDER_ALERT`
- `MESSAGE_ALERT`
- `CALLING`
- `ERROR`
- `FACTORY_TEST`

主要转换：

- `BOOT -> IDLE`：配置、存储、服务初始化成功。
- `IDLE -> REMINDER_ALERT`：提醒到时。
- `REMINDER_ALERT -> IDLE`：老人点击“我知道了”并确认提醒。
- `IDLE -> MESSAGE_ALERT`：收到新文字留言。
- `MESSAGE_ALERT -> IDLE`：未读留言被播放或标记已读。
- `IDLE -> CALLING`：触发一键呼叫。
- `CALLING -> IDLE`：mock 呼叫记录完成。
- `* -> ERROR`：启动或运行异常。
- `* -> FACTORY_TEST`：工厂测试流程。

所有状态变化都会写入 `data/event_log.txt`。
