# Elder Care Terminal Architecture

## Layers
- `core/`: Logger, state machine, reminder polling thread.
- `services/`: Data persistence and audio service.
- `hardware/`: `IAudio`/`ICamera`/`IGpio`, mock implementations, RK3566 placeholder.
- `web/`: Minimal HTTP server and static front-end assets.

## State machine
States:
- `BOOT`
- `IDLE`
- `REMINDER_ALERT`
- `MESSAGE_ALERT`
- `CALLING`
- `ERROR`
- `FACTORY_TEST`

All transitions are written to `data/event_log.txt`.

### Transition examples
- `BOOT -> IDLE`: after initialization.
- `IDLE -> REMINDER_ALERT`: reminder polling detects due/unconfirmed reminder.
- `ANY -> ERROR`: hardware/service failure.
- `IDLE -> MESSAGE_ALERT`: new message event.
- `IDLE -> CALLING`: outgoing/incoming call flow.
- `IDLE -> FACTORY_TEST`: enter factory verification mode.
- `REMINDER_ALERT -> IDLE`: reminder acknowledged.

## Reminder polling
A background thread polls every 10 seconds.
- On first detection of an unacknowledged reminder: transition to `REMINDER_ALERT` and call `AudioService::playAlert()`.
- If not acknowledged after 180 seconds: append timeout log entry.

## RK3566 TODO extension points
- 4G/SIP/WebRTC signaling and media channel.
- V4L2/OpenCV camera capture pipeline.
- ALSA/aplay audio playback.
