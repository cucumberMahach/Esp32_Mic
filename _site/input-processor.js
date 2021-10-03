class InputProcessor extends AudioWorkletProcessor {

    buff = []

    constructor() {
        super();
        this.port.onmessage = event => {
            let s = event.data.sound;
            if (this.buff.length > 128 * 30)
                this.buff.splice(0, this.buff.length);
            for (let i = 0; i < s.length; i++){
                this.buff.push(s[i]);
            }
        }

        console.log("constructor");
    }


    process(inputs, outputs, parameters) {
        const output = outputs[0]
        const input = inputs[0]
        let size = 128;
        if (this.buff.length >= size) {
            for (let ch = 0; ch < input.length; ch++) {
                let i = input[ch];
                let o = output[ch];
                for (let j = 0; j < i.length; j++) {
                    o[j] = this.buff[j];
                }
            }
            this.buff.splice(0, size);
        } else {
            for (let ch = 0; ch < input.length; ch++) {
                let i = input[ch];
                let o = output[ch];
                for (let j = 0; j < i.length; j++) {
                    o[j] = 0;
                }
            }
        }
        return true
    }
}

registerProcessor('input-processor', InputProcessor)