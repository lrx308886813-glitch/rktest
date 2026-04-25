# HTTP API

默认监听 `http://127.0.0.1:8080`。

- `GET /api/health`
- `GET /api/device/status`
- `GET /api/reminders`
- `POST /api/reminders`
- `POST /api/reminders/ack?id=xxx`
- `GET /api/messages`
- `POST /api/messages`
- `POST /api/messages/read?id=xxx`
- `GET /api/contacts`
- `POST /api/contacts`
- `POST /api/call/primary`
- `GET /api/logs/recent`

请求体使用简单 JSON，例如：

```json
{"title":"下午散步","type":"EXERCISE","time":"16:00","enabled":true}
```

第一版 API 没有鉴权，部署到家庭网络外之前必须增加认证、TLS 或网关保护。
