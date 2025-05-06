const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 3000;

// 提供靜態檔案（HTML）
app.use(express.static(path.join(__dirname, 'public')));
app.use(express.urlencoded({ extended: true }));
app.use(express.json());

const MOTOR_DEVICE = '/dev/motor_ctrl';

app.post('/control', (req, res) => {
  const { command } = req.body;
  console.log(`Received command: ${command}`);

  // 寫入 motor 控制指令
  fs.writeFile(MOTOR_DEVICE, command.toString(), (err) => {
    if (err) {
      console.error('Failed to write to motor device:', err);
      return res.status(500).send('Failed to control motor');
    }
    res.send('OK');
  });
});

app.listen(port, () => {
  console.log(`🚗 Car control server running at http://localhost:${port}`);
});

