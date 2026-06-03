# EdgeCam

家用室外瓦斯錶邊緣照相機，使用 ESP32-CAM、Wi-Fi 與 Blynk 手動觸發拍照，拍照後透過 HTTP multipart 上傳圖片並回報狀態，完成後進入低功耗睡眠。

## 目前功能

- ESP32-CAM AI Thinker 相機初始化與 JPEG 拍照
- Blynk Virtual Pin 手動拍照命令
- Wi-Fi 連線、逾時與重試
- HTTP multipart 圖片上傳接口
- Blynk 狀態、圖片 URL、電池電壓、RSSI、錯誤碼回報
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
- HTTP 圖片上傳 API
- 裝置 ID
- 睡眠與重試參數

## Blynk Virtual Pins

| Pin | 方向 | 說明 |
|---|---|---|
| V0 | App -> Device | 拍照命令，送 `1` |
| V1 | Device -> App | 裝置狀態 |
| V2 | Device -> App | 上次拍照時間 |
| V3 | Device -> App | 圖片 URL |
| V4 | Device -> App | 電池電壓 |
| V5 | Device -> App | Wi-Fi RSSI |
| V6 | Device -> App | 錯誤碼 |
| V7 | App -> Device | 立即睡眠，送 `1` |
| V8 | App -> Device | 補光手動控制，`0`/`1` |
| V9 | App -> Device | 重啟裝置，送 `1` |

## HTTP 圖片上傳接口

`POST /api/v1/devices/{device_id}/captures`

Content-Type: `multipart/form-data`

欄位：

| 欄位 | 型別 | 必填 | 說明 |
|---|---|---|---|
| image | file | 是 | JPEG 圖片 |
| device_id | string | 是 | 裝置 ID |
| captured_at | string | 是 | ISO 8601 時間 |
| battery_voltage | number | 否 | 電池電壓 |
| wifi_rssi | number | 否 | Wi-Fi 訊號 |
| firmware_version | string | 否 | 韌體版本 |

回應建議：

```json
{
  "success": true,
  "capture_id": "cap_20260603_001",
  "image_url": "https://example.com/images/gas-meter-cam-001/20260603-203000.jpg",
  "received_at": "2026-06-03T20:30:05+08:00"
}
```

韌體會嘗試從回應 JSON 中擷取 `image_url` 並寫回 Blynk `V3`。

## 低功耗運作方式

ESP32 深度睡眠時無法即時接收 Blynk 命令，所以第一版採用兩種模式：

- 定時醒來：每隔一段時間醒來連線，保留 `COMMAND_WINDOW_MS` 等待 App 命令，逾時睡眠。
- 外部喚醒：按下本地按鍵或由外部電路喚醒後，可設定為立即拍照。

若需要「每月任意時間按 App 都能立刻拍照」，裝置必須常在線，或增加外部低功耗喚醒硬體。
