const express = require('express');
const net = require('net');
const app = express();

const port = 3003;
const LB_CONTROL_PORT = 8081;

app.get("/",(req, res)=>{
    res.send(`Hello There ${port}`);
});

app.listen(port, ()=>{
    console.log(`Node server on port ${port}`);
    const client = new net.Socket();

    client.connect({port:LB_CONTROL_PORT,host: '0.0.0.0',family:4},()=>{
        client.write(`REGISTER 127.0.0.1:${port}\n`);
    });

 // client.closed()

    client.on('error', (err) => {
        console.error("Could not register to C++ Load Balancer. Is it running?", err.message);
    });
})