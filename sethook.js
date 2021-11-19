const RippleAPI = require('ripple-lib').RippleAPI;
const fs = require('fs');
const api = new RippleAPI({ server: 'wss://hooks-testnet.xrpl-labs.com' });

const cfgPath = 'hook.cfg';
let cfg;

if (!fs.existsSync(cfgPath)) {
    cfg = {
        address: "",
        secret: ""
    }
    fs.writeFileSync(cfgPath, JSON.stringify(cfg, null, 2));
}
else {
    cfg = JSON.parse(fs.readFileSync(cfgPath));
}

if (cfg.address === "" || cfg.secret === "") {
    console.log("SETHOOK FAILED: Please specify hook address and secret in hook.cfg");
}
else {
    const wasmfile = process.argv[2] || "build/evernode.wasm";
    const secret = cfg.secret;
    const address = api.deriveAddress(api.deriveKeypair(secret).publicKey);

    console.log("SetHook on address " + address);

    api.on('error', (errorCode, errorMessage) => { console.log(errorCode + ': ' + errorMessage); });
    api.on('connected', () => { console.log('connected'); });
    api.on('disconnected', (code) => { console.log('disconnected, code:', code); });
    api.connect().then(() => {
        binary = fs.readFileSync(wasmfile).toString('hex').toUpperCase();
        j = {
            Account: address,
            TransactionType: "SetHook",
            CreateCode: binary,
            HookOn: '0000000000000000'
        }
        api.prepareTransaction(j).then((x) => {
            let s = api.sign(x.txJSON, secret)
            api.submit(s.signedTransaction).then(response => {
                console.log(response.resultCode, response.resultMessage);
                process.exit(0);
            }).catch(e => { console.log(e) });
        });
    }).then(() => { }).catch(console.error);
}