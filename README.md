# EdgeCam

ESP32-CAM 家用室外瓦斯錶拍照節點。預設硬體包含 `TPL5110` 低功耗電源控制模組與 `DS3231 RTC`，用 Blynk 的 `need_picture` 旗標決定是否拍照，照片透過 Telegram Bot 回傳。

## 預設硬體

目前程式預設你有接：

- ESP32-CAM AI Thinker
- TPL5110 Power Timer
- DS3231 RTC
- 5V 升壓模組
- 18650 電池組

預設流程：

```text
TPL5110 定時開電
→ ESP32-CAM 開機
→ 讀 RTC
→ 不在 08:00-16:00：拉 DONE，TPL5110 關電
→ 在 08:00-16:00：連 Wi-Fi / Blynk
→ 同步 V0 need_picture
→ false：回報後拉 DONE 關電
→ true：自動補光拍照，Telegram 傳圖，清 V0=false，拉 DONE 關電
```

## 沒有 TPL5110 / RTC 時要改哪裡

請編輯 [include/config.h](include/config.h)。

如果沒有 TPL5110，把這行：

```cpp
#define EXTERNAL_POWER_CONTROL_ENABLED 1
```

改成：

```cpp
#define EXTERNAL_POWER_CONTROL_ENABLED 0
```

如果沒有 RTC，把這兩行：

```cpp
#define RTC_ENABLED 1
#define ACTIVE_WINDOW_ENABLED 1
```

改成：

```cpp
#define RTC_ENABLED 0
#define ACTIVE_WINDOW_ENABLED 0
```

如果沒有 RTC，且想改用 ESP32 deep sleep 的外部喚醒腳，再把這行改成 `1`：

```cpp
#define DEEP_SLEEP_EXT_WAKEUP_ENABLED 1
```

注意：目前 `GPIO13` 預設給 RTC `SCL` 使用，所以 `DEEP_SLEEP_EXT_WAKEUP_ENABLED` 預設關閉。

## 接線摘要

```text
18650 電池組 +
  → TPL5110 VDD

18650 電池組 -
  → TPL5110 GND
  → 升壓模組 IN-
  → ESP32-CAM GND

TPL5110 DRV / LOAD+
  → 升壓模組 IN+

升壓模組 OUT+
  → ESP32-CAM 5V

升壓模組 OUT-
  → ESP32-CAM GND

ESP32-CAM GPIO14
  → TPL5110 DONE

RTC VCC
  → ESP32-CAM 3.3V

RTC GND
  → ESP32-CAM GND

RTC SDA
  → ESP32-CAM GPIO15

RTC SCL
  → ESP32-CAM GPIO13
```

## 重要設定

[include/config.h](include/config.h) 內主要設定：

```cpp
#define EXTERNAL_POWER_CONTROL_ENABLED 1
#define POWER_DONE_PIN 14

#define RTC_ENABLED 1
#define RTC_TYPE RTC_TYPE_DS3231
#define RTC_SDA_PIN 15
#define RTC_SCL_PIN 13
#define ACTIVE_WINDOW_ENABLED 1
#define ACTIVE_START_HOUR 8
#define ACTIVE_END_HOUR 16

#define DEEP_SLEEP_EXT_WAKEUP_ENABLED 0
```

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

V8 手動補光已移除。補光只在拍照流程中自動開啟，拍完自動關閉。

## Telegram Bot 設定

1. 在 Telegram 找 `@BotFather`。
2. 建立 bot，取得 `TELEGRAM_BOT_TOKEN`。
3. 用你的 Telegram 帳號傳一則訊息給該 bot。
4. 開啟下列網址取得 `chat.id`：

```text
https://api.telegram.org/bot<TELEGRAM_BOT_TOKEN>/getUpdates
```

5. 將 `chat.id` 填入 [include/config.h](include/config.h) 的 `TELEGRAM_CHAT_ID`。

## 開發指令

建議使用 PlatformIO。

```powershell
pio run
pio run -t upload
pio device monitor
```

## 目前限制

- 深度睡眠或 TPL5110 關電期間，ESP32-CAM 無法即時收到 Blynk 命令。
- TPL5110 只負責週期開電，不知道現在幾點；RTC 是在 ESP32-CAM 醒來後判斷是否處於 `08:00-16:00`。
- 若要硬體層級完全避免非時段喚醒，需要 RTC alarm 另外接外部電源控制電路。
- Telegram 上傳有 `TELEGRAM_MAX_PHOTO_BYTES` 限制，避免圖片過大造成記憶體或傳輸不穩。
