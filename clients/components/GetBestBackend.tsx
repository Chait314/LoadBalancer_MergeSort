import net from 'net';

const PORT = 8080
const IP = '0.0.0.0'

const GetBestBackend = () => {
  return new Promise((resolve, reject)=> {
    const client = new net.Socket();
    client.connect({port: PORT, host: IP, family: 4}, ()=> {
        console.log('connecting to c++ loadbalancer');
    });

    client.on('data', (data)=>{
        const response = data.toString().trim();
        client.destroy();
        if(response.startsWith("REDIRECT")){
            const [_, target] = response.split(' ');
            const [ip, port] = target.split(':');
            resolve({ip, port: parseInt(port)});
        }
        else{
            reject(new Error('Invalid protocol response from balancer'));
        }
    });

    client.on('error', (err) => {
        reject(err);
    });
  });
}

export default GetBestBackend;
