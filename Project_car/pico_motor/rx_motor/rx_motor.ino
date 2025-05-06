#include <IRremote.h>

#define IR_RECEIVE_PIN 15  // 這裡改成你接的腳位

void setup() {
  Serial2.setTX(4);      // UART1 TX 腳位（GPIO4）
 // Serial1.setRX(5);      // UART1 RX 腳位（GPIO5）
  Serial2.begin(9600);   // 開啟硬體 UART
  Serial.begin(9600);
  delay(2000); // 等待 Serial 穩定
  Serial.println("紅外線接收器啟動中...");
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // 啟動接收器
}

void loop() {
  if (IrReceiver.decode()) {
    //Serial.println("接收到紅外線訊號！");
    //IrReceiver.printIRResultShort(&Serial); // 簡單列出接收到的資訊
    //IrReceiver.printIRSendUsage(&Serial);   // 印出重播指令
    Serial.println(IrReceiver.decodedIRData.command, HEX);

  String command ="STOP";


  if (IrReceiver.decodedIRData.command==0xF) {
    command = "F";
    Serial.println("DIRECTION: FORWARD");
  } else if (IrReceiver.decodedIRData.command==0xF0) {
    command = "B";
    Serial.println("DIRECTION: BACKWARD");
  } else if (IrReceiver.decodedIRData.command==0xC3) {
    command = "L";
    Serial.println("DIRECTION: LEFT");
  } else if (IrReceiver.decodedIRData.command==0x3C) {
    command = "R";
    Serial.println("DIRECTION: RIGHT");
  } else if (IrReceiver.decodedIRData.command==0xAA) {
    command = "S";
    Serial.println("STOP");
  } else {
    Serial.println("UNKNOWN");
  }

  IrReceiver.resume(); // 準備接收下一個訊號

  Serial2.println(command);
  }
  delay(100); // 每100ms更新一次
}
