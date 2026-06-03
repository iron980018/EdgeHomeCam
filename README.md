# EdgeCam

家用室外瓦斯錶邊緣照相機，使用 ESP32-CAM、Wi-Fi、Blynk 與 Telegram Bot。Blynk 負責設定拍照需求與狀態顯示，Telegram Bot 負責把拍攝照片直接傳到指定聊天室。

## 目前功能

- ESP32-CAM AI Thinker 相機初始化與 JPEG 拍照
- Blynk `need_picture` 雲端旗標觸發拍照
- Wi-Fi 連線、逾時與重試
- Telegram Bot `sendPhoto` 圖片回傳
- Telegram 圖片 caption 附帶裝置、時間、電池、RSSI、韌體版本
- Blynk 狀態、Telegram 訊息標記、電池電壓、RSSI、錯誤碼回報
- 拍照成功後自動將 `need_picture` 改回 `false`
- 拍照或傳送失敗時保留 `need_picture=true`，下次醒來可再重試
- 拍照時自動開啟補光 LED，拍完自動關閉
- Deep Sleep 低功耗模式
- 定時醒來後同步 Blynk 狀態
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

## 操作流程

1. 在 Blynk App 上把 `need_picture` 設為 `true`。
2. ESP32-CAM 下一次醒來後連上 Wi-Fi 與 Blynk。
3. 韌體執行 `Blynk.syncVirtual(V0)`，同步雲端 `need_picture` 值。
4. 如果 `need_picture=true`，裝置拍照並用 Telegram Bot 傳送照片。
5. Telegram 傳送成功後，裝置把 Blynk `need_picture` 寫回 `false`。
6. 裝置回報狀態後進入 Deep Sleep。

注意：ESP32-CAM 在 Deep Sleep 時無法即時收到 Blynk 命令。手機設定 `need_picture=true` 後，裝置會在下一次醒來時才處理。

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

| Pin | 方向 | 名稱 | 說明 |
|---|---|---|---|
| V0 | App <-> Device | `need_picture` | App 設為 `1/true` 代表需要拍照；成功後裝置寫回 `0/false` |
| V1 | Device -> App | `status` | 裝置狀態 |
| V2 | Device -> App | `last_capture_at` | 上次拍照時間 |
| V3 | Device -> App | `telegram_result` | Telegram 傳送結果，例如 `telegram:message/123` |
| V4 | Device -> App | `battery_voltage` | 電池電壓 |
| V5 | Device -> App | `wifi_rssi` | Wi-Fi RSSI |
| V6 | Device -> App | `error_code` | 錯誤碼 |
| V7 | App -> Device | `sleep_now` | 立即睡眠，送 `1` |
| V9 | App -> Device | `reboot` | 重啟裝置，送 `1` |

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

ESP32 深度睡眠時無法即時接收 Blynk 命令，所以第一版採用「雲端旗標」模式：

- 使用者在 App 先把 `need_picture` 設為 `true`。
- 裝置定時醒來並同步 Blynk `V0`。
- 若 `V0=true`，拍照並傳 Telegram。
- 成功後裝置將 `V0=false`。
- 若 `V0=false`，等待短時間後回到睡眠。

若需要更長續航，建議未來加入外部低功耗電源控制，讓 ESP32-CAM 在睡眠期間整機斷電。
