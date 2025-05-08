// Pico W 搖桿讀取並顯示訊號

// 設定搖桿腳位
const int pinX = A0; // 搖桿 X軸接 A0
const int pinY = A1; // 搖桿 Y軸接 A1
const int pinK = 2;  // 搖桿 按鈕接 D2

// 搖桿範圍設定
const int centerMin = 450; // 中心區域下限
const int centerMax = 570; // 中心區域上限

void setup() {
  Serial2.setTX(4);      // UART1 TX 腳位（GPIO4）
 // Serial1.setRX(5);      // UART1 RX 腳位（GPIO5）
  Serial2.begin(9600);   // 開啟硬體 UART
  Serial.begin(9600); // 開啟 Serial 通訊，用於監控顯示
  pinMode(pinK, INPUT_PULLUP); // 按鈕是接地觸發
}

void loop() {
  int xVal = analogRead(pinX); // 讀取 X 軸
  int yVal = analogRead(pinY); // 讀取 Y 軸
  int kVal = digitalRead(pinK); // 讀取按鈕

  // 顯示 X、Y 軸數值和按鈕狀態
  Serial.print("X: "); 
  Serial.print(xVal); 
  Serial.print("\t");

  Serial.print("Y: ");
  Serial.print(yVal);
  Serial.print("\t");

  // 先預設沒有動作
  String command = "STOP";

  // 判斷上下左右
  Serial.print("DIRECTION: ");
  if (yVal > centerMax) {
    command = "F";
    Serial.println("FORWARD");
  } else if (yVal < centerMin) {
    command = "B";
    Serial.println("BACKWARD");
  } else if (xVal > centerMax) {
    command = "L";
    Serial.println("LEFT");
  } else if (xVal < centerMin) {
    command = "R";
    Serial.println("RIGHT");
  } else {
    command = "S";
    Serial.println("STOP");
  }

  // 如果按鈕有按下去（低電位）
  Serial.print("Button: ");
  if (kVal == LOW) {
    command = "S";
    Serial.println("PRESSED");
  } else {
    Serial.println("RELEASED");
  }

  // 傳送指令給 RPi5
  Serial2.println(command);

  delay(100); // 每100ms更新一次
}
