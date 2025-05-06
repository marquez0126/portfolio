const express = require('express');
const { exec } = require('child_process');
const path = require('path');
const app = express();
const port = 3000;

const allowed = ['F', 'B', 'L', 'R', 'S'];

// Serve static index.html
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// Command route
app.get('/:cmd', (req, res) => {
    const cmd = req.params.cmd.toUpperCase();
    if (!allowed.includes(cmd)) {
        return res.status(400).send('Invalid command');
    }

    exec(`./motor_control ${cmd}`, (error, stdout, stderr) => {
        if (error) {
            console.error(`Exec error: ${error}`);
            return res.status(500).send('Execution error');
        }
        console.log(`Motor command executed: ${cmd}`);
        res.send(`Motor command executed: ${cmd}`);
    });
});

app.listen(port, () => {
    console.log(`Motor control server running at http://localhost:${port}`);
});

