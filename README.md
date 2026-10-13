# ESP32 交通號誌實驗室

CIRCUS Pi EZ Start Kit+ for ESP32 / NodeMCU-32S 的單組紅黃綠燈教學專案。

## 功能

- 完整繁體中文網頁，適用手機與電腦；無外部 CDN，不需網際網路。
- 自動循環：紅 → 綠 → 黃 → 紅；預設紅 10 秒、綠 10 秒、黃 3 秒。
- 網頁與 OLED 同步倒數，每階段可設 1–120 秒。
- 手動模式保持指定燈色；倒數顯示「—」，OLED 顯示 MANUAL / HOLD。
- 停止時保持紅燈；開機預設停止。啟動、切換模式、套用秒數都回到紅燈。
- ESP32 使用非阻塞計時，瀏覽器關閉或斷線時，自動循環仍由 ESP32 執行。
- 斷線會停用網頁操作，不把模擬結果當作實體控制成功。
- index.html 提供離線模擬，可直接下載開啟或使用 GitHub Pages。

## 套件與腳位

不用另外接三顆 LED；使用擴充板既有的燈與 OLED。將 NodeMCU-32S **依板上方向標示**插入 ESP32 專用插槽，不可反插；斷電後再插拔，依套件手冊供電並開啟擴充板開關。

| 元件 | ESP32 GPIO |
| --- | --- |
| 紅 LED | 16 |
| 黃 LED | 12 |
| 綠 LED | 13 |
| OLED SDA | 21 |
| OLED SCL | 22 |

腳位依據 [官方產品資料](https://www.icshop.com.tw/products/368030200677)及[官方腳位對照圖](https://w3.icshop.com.tw/pd/368030200655/Q2.jpg)。板子版本不同時以實際說明書為準。

OLED 預設 SH1106、128×64、I²C 位址 0x3C；官方商品文字只標示尺寸，**顯示器控制晶片與位址仍需實機確認**。若畫面空白，先檢查供電，將 config.h 的 OLED_SH1106 改為 0 試 SSD1306；若你的螢幕是 0x3D，修改 OLED_ADDRESS。LED 預設高電位點亮，版本若相反，修改 LED_ACTIVE_HIGH。

## Arduino IDE 上傳步驟

1. 安裝 Arduino IDE。在偏好設定的額外開發板管理員網址加入 `https://espressif.github.io/arduino-esp32/package_esp32_index.json`。
2. 在開發板管理員安裝 **esp32 by Espressif Systems**。
3. 在程式庫管理員安裝 **U8g2 by oliver**。
4. 在電腦建立名為 **TrafficLight** 的資料夾，把 `TrafficLight.ino`、`config.h`、`web_ui.h` 放在同一個資料夾。開啟 TrafficLight.ino。
5. 選擇 **NodeMCU-32S** 開發板與正確 COM 埠；使用可傳輸資料的 USB 線。若沒有 COM 埠，依套件 USB 晶片安裝官方 CP210x 驅動。
6. 建議先在 config.h 修改 AP_PASSWORD（至少 8 字元），再按驗證、上傳。若停在 Connecting，依開發板說明按住 BOOT 直到開始寫入。
7. 打開序列監控視窗，鮑率設定為 **115200**。重開機後應顯示 Wi-Fi 名稱與網址。
8. 手機或電腦連到 **ESP32-Traffic-Lab**；教學預設密碼為 **traffic123**。若手機顯示「沒有網際網路」，選擇維持連線。
9. 瀏覽器開啟 `http://192.168.4.1`。必須看到「ESP32 已連線」才是實體控制。按「啟動」開始。

若不使用 OLED，可將 ENABLE_OLED 設為 0，就不需要 U8g2 程式庫。

## 操作練習

1. 自動模式按啟動，觀察紅 → 綠 → 黃與倒數。
2. 改成紅 5、綠 5、黃 2 秒，按套用；正在運行時立刻從紅燈重算。
3. 切換手動模式，按啟動，再選擇燈色；任何時刻只有一種燈亮。
4. 按停止，應保持紅燈。停止後必須先啟動才能手動選燈。
5. 自動模式關掉網頁，確認實體仍循環；重開頁面會重新取得 ESP32 狀態。

## 網頁與 GitHub 的關係

GitHub 儲存程式碼；GitHub Pages 的 HTTPS 網頁提供**模擬**。實體控制網頁由 ESP32 直接提供，與 API 同來源，因此不用跨來源權限，也避免 HTTPS 網頁連到區域 HTTP 裝置的限制。不要在 GitHub 網頁的模擬模式下期待實體 LED 改變。

`index.html` 與韌體內 `web_ui.h` 是相同控制介面，韌體版本加入 `window.TRAFFIC_DEVICE=true` 以啟用裝置 API。修改介面後需同步更新 header 並重新上傳韌體。

## API

| 方法 / 路徑 | 用途 |
| --- | --- |
| GET /api/state | 取得 mode、running、light、remaining 與 durations |
| POST /api/action | 以 application/x-www-form-urlencoded 送出 command |

command 可為 start、stop、mode（value=auto/manual）、light（value=red/yellow/green）、config（red/green/yellow 為 1–120 的整數）。無效輸入回傳 400；未啟動手動模式時選燈回傳 409。設定僅存在記憶體，重開機恢復預設。

Wi-Fi 密碼限制加入此教學網路；任何已加入的裝置皆可操作控制台。此為單組號誌學習模型，不包含真實路口雙向互鎖或道路設施功能。

## 驗證與限制

已測試網頁模擬狀態切換、循環、倒數、停止與輸入限制。此環境未連接你的套件；Arduino 編譯與 LED / OLED 實機驗證請依上述步驟完成。

參考：[Espressif Wi-Fi API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)、[官方 WebServer 範例](https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/examples/WebServer/WebServer.ino)、[U8g2 顯示器設定](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp)。
