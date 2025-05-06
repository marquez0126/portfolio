#include <IRremote.h>

#define IR_RECEIVE_PIN 15  // 這裡改成你接的腳位

void setup() {
  Serial.begin(115200);
  delay(2000); // 等待 Serial 穩定
  Serial.println("紅外線接收器啟動中...");
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // 啟動接收器
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println("接收到紅外線訊號！");
    IrReceiver.printIRResultRawFormatted(&Serial, true); // 印出完整 raw 資料
    
    IrReceiver.resume(); // 準備接收下一個訊號
  }
}

