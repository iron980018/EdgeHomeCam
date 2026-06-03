# EdgeCam Architecture

本文件整理目前 EdgeCam 的整體架構、資料流、介面分工，以及後續建議修改項目。

## System Goal

EdgeCam 是家用室外瓦斯錶拍照節點。使用者在 Blynk App 設定 `need_picture=true`，裝置下一次醒來後檢查旗標，需要拍照時才拍照並透過 Telegram Bot 回傳照片。成功後裝置把 `need_picture=false`，再進入低功耗狀態。

## High-Level Flow

```text
Blynk App
  -> set V0 need_picture=true

ESP32-CAM wake
  -> connect Wi-Fi
  -> connect Blynk
  -> sync V0 need_picture
  -> if false: report status and sleep
  -> if true:
       initialize camera
       turn on flash LED
       capture JPEG
       turn off flash LED
       send photo to Telegram
       if success:
         write V0 need_picture=false
       report status
       sleep
```

## Hardware Architecture

目前韌體可先使用 ESP32-CAM 自身 deep sleep。若要長時間續航，建議使用外部電源切斷模組。

```text
18650 battery pack
  -> TPL5110 power timer
  -> 5V boost converter
  -> ESP32-CAM
  -> GPIO POWER_DONE_PIN, future
  -> TPL5110 DONE
```

目前程式尚未加入 `POWER_DONE_PIN`。加入後，裝置完成任務時應拉高 DONE，讓 TPL5110 切斷整機電源。

## Firmware Modules

| Module | File | Responsibility |
|---|---|---|
| Main flow | `src/main.cpp` | 開機流程、狀態切換、拍照上傳流程、睡眠流程 |
| Camera | `src/camera_manager.*` | ESP32-CAM 初始化、拍照、釋放 frame buffer |
| Wi-Fi | `src/wifi_manager.*` | Wi-Fi 連線、重試、RSSI |
| Blynk | `src/blynk_service.*` | Blynk 連線、同步 `need_picture`、狀態回報 |
| Telegram upload | `src/upload_service.*` | Telegram Bot `sendPhoto` multipart 上傳 |
| Power | `src/power_manager.*` | GPIO 初始化、電池電壓、deep sleep |
| Config | `include/config.h` | Wi-Fi、Blynk、Telegram、睡眠、GPIO 設定 |

## Blynk Interface

| Pin | Name | Direction | Current Behavior |
|---|---|---|---|
| V0 | `need_picture` | App <-> Device | App 設 `1`，裝置成功拍照後清為 `0` |
| V1 | `status` | Device -> App | `booting`, `connecting`, `idle`, `capturing`, `uploading`, `sleeping`, `error` |
| V2 | `last_capture_at` | Device -> App | 上次拍照時間 |
| V3 | `telegram_result` | Device -> App | `telegram:message/<id>` 或 `telegram:sent` |
| V4 | `battery_voltage` | Device -> App | 電池電壓，若未啟用 ADC 會是 `0.0` |
| V5 | `wifi_rssi` | Device -> App | Wi-Fi 訊號強度 |
| V6 | `error_code` | Device -> App | 最近一次錯誤碼 |
| V7 | `sleep_now` | App -> Device | 立即睡眠 |
| V9 | `reboot` | App -> Device | 重啟裝置 |

V8 手動補光已移除。補光只在拍照流程中自動開啟，拍完自動關閉。

## Telegram Interface

```http
POST https://api.telegram.org/bot<TELEGRAM_BOT_TOKEN>/sendPhoto
Content-Type: multipart/form-data
```

Fields:

| Field | Type | Description |
|---|---|---|
| `chat_id` | string | Telegram chat ID |
| `photo` | file | JPEG image from ESP32-CAM |
| `caption` | string | Device ID, capture time, battery, RSSI, firmware version |

## Current Power Behavior

目前版本：

```text
task complete
  -> publish sleeping status
  -> flush Blynk loop
  -> delay SLEEP_AFTER_TASK_SEC
  -> ESP32 deep sleep
```

長續航建議版本：

```text
task complete
  -> publish sleeping status
  -> flush Blynk loop
  -> set POWER_DONE_PIN high
  -> TPL5110 cuts ESP32-CAM power
```

## Recommended Next Changes

| Priority | Item | Reason |
|---|---|---|
| High | Add `POWER_DONE_PIN` support | 搭配 TPL5110 真正切斷 ESP32-CAM 電源，提升續航 |
| High | Add config validation | 避免 Wi-Fi/Blynk/Telegram token 未填時反覆重試耗電 |
| High | Reduce camera initialization when no picture needed | 若 `need_picture=false`，可不初始化相機，省醒來時間與電 |
| Medium | Add Telegram response parsing for `ok=false` | Telegram HTTP 200 但 API 失敗時應回報更準確錯誤 |
| Medium | Add battery ADC calibration | 目前電池電壓功能預設關閉，需要依分壓電阻校準 |
| Medium | Add capture settings in config | 補光等待時間、畫質、解析度可集中調整 |
| Low | Add retry/backoff for Telegram send | 網路短暫不穩時提高成功率 |
| Low | Add build check workflow | GitHub Actions 或本機 PlatformIO 檢查 |

## Notes

- Deep sleep 期間無法即時接收 Blynk 指令；目前設計是醒來後同步雲端旗標。
- `need_picture=true` 會保留到成功拍照並傳 Telegram 後才清除，失敗時下次醒來會再試。
- 若使用 TPL5110，韌體不應只 deep sleep，而應拉高 DONE 讓外部電源切斷。
