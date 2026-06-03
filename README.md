# EdgeCam

家用室外瓦斯錶邊緣照相機，使用 ESP32-CAM、Wi-Fi、Blynk 與 Telegram Bot。Blynk 負責手動觸發與狀態顯示，Telegram Bot 負責把拍攝照片直接傳到指定聊天室。

## 目前功能

- ESP32-CAM AI Thinker 相機初始化與 JPEG 拍照
- Blynk Virtual Pin 手動拍照命令
- Wi-Fi 連線、逾時與重試
- Telegram Bot `sendPhoto` 圖片回傳
- Telegram 圖片 caption 附帶裝置、時間、電池、RSSI、韌體版本
- Blynk 狀態、Telegram 訊息標記、電池電壓、RSSI、錯誤碼回報
- 補光 LED 控制
- Deep Sleep 低功耗模式
- 定時喚醒後保留短暫線上等待命令視窗
- 本地按鍵/外部腳位喚醒後可自動拍照

## 開發環境

建議使用 PlatformIO。

```powershell
pio run
pio run -t upload
pio device monitor
```

## 設定

請編輯 [include/config.h](include/config.h)，填入：

- Wi-Fi SSID / 密碼
- Blynk Template ID / Template Name / Auth Token
- Telegram Bot Token
- Telegram Chat ID
- 裝置 ID
- 睡眠與重試參數

## Telegram Bot 設定

1. 在 Telegram 找 `@BotFather`。
2. 建立 bot，取得 `TELEGRAM_BOT_TOKEN`。
3. 用你的 Telegram 帳號傳一則訊息給該 bot。
4. 開啟下列網址取得 `chat.id`：

```text
https://api.telegram.org/bot<TELEGRAM_BOT_TOKEN>/getUpdates
```

5. 將 `chat.id` 填入 [include/config.h](include/config.h) 的 `TELEGRAM_CHAT_ID`。

若要傳到群組，請先把 bot 加入群組，再從 `getUpdates` 找出群組的 `chat.id`。群組 ID 通常是負數。

## Blynk Virtual Pins

| Pin | 方向 | 說明 |
|---|---|---|
| V0 | App -> Device | 拍照命令，送 `1` |
| V1 | Device -> App | 裝置狀態 |
| V2 | Device -> App | 上次拍照時間 |
| V3 | Device -> App | Telegram 傳送結果，例如 `telegram:message/123` |
| V4 | Device -> App | 電池電壓 |
| V5 | Device -> App | Wi-Fi RSSI |
| V6 | Device -> App | 錯誤碼 |
| V7 | App -> Device | 立即睡眠，送 `1` |
| V8 | App -> Device | 補光手動控制，`0`/`1` |
| V9 | App -> Device | 重啟裝置，送 `1` |

## Telegram 接口格式

韌體使用 Telegram Bot API：

```http
POST https://api.telegram.org/bot<TELEGRAM_BOT_TOKEN>/sendPhoto
Content-Type: multipart/form-data
```

欄位：

| 欄位 | 型別 | 必填 | 說明 |
|---|---|---|---|
| chat_id | string | 是 | Telegram 使用者、群組或頻道 ID |
| photo | file | 是 | ESP32-CAM 拍攝的 JPEG |
| caption | string | 否 | 裝置狀態摘要 |

目前 caption 格式：

```text
EdgeCam gas meter capture
Device: gas-meter-cam-001
Time: 2026-06-03T20:30:00+0800
Battery: 3.92 V
RSSI: -67 dBm
Firmware: 0.1.0
```

成功時 Telegram 會回傳 JSON，其中包含 `result.message_id`。韌體會擷取 `message_id`，並把 `telegram:message/<message_id>` 寫回 Blynk `V3`。

## 錯誤碼

| 錯誤碼 | 說明 |
|---|---|
| WIFI_TIMEOUT | Wi-Fi 連線逾時 |
| BLYNK_TIMEOUT | Blynk 連線逾時 |
| CAMERA_INIT_FAILED | 相機初始化失敗 |
| CAPTURE_FAILED | 拍照失敗 |
| TELEGRAM_BEGIN_FAILED | Telegram HTTP client 初始化失敗 |
| TELEGRAM_SEND_FAILED | Telegram API 傳送失敗 |
| TELEGRAM_OUT_OF_MEMORY | 建立 multipart body 時記憶體不足 |
| LOW_BATTERY | 電池電壓過低 |

## 低功耗運作方式

ESP32 深度睡眠時無法即時接收 Blynk 命令，所以第一版採用兩種模式：

- 定時醒來：每隔一段時間醒來連線，保留 `COMMAND_WINDOW_MS` 等待 App 命令，逾時睡眠。
- 外部喚醒：按下本地按鍵或由外部電路喚醒後，可設定為立即拍照。

若需要「每月任意時間按 App 都能立刻拍照」，裝置必須常在線，或增加外部低功耗喚醒硬體。
