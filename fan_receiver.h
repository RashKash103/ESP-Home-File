#define RECEIVER_PIN 5
#define ON_PERSISTENCE 1
#define OFF_PERSISTENCE 2

#include "esphome.h"

class Mapping {
    public: 
    Mapping(BinarySensor* _sensor, String _commandName, int _value);

    BinarySensor* sensor;   // binary sensor object associated
    String commandName;     // name of the associated command
    int value;              // value to query against

    bool previousState;     // previous state of the button
    bool state;             // current state of the button

    int onCount;            // persistence when pressing
    int offCount;           // persistence when releasing
};

Mapping::Mapping(BinarySensor* _sensor, String _commandName, int _value) {
        sensor = _sensor;
        commandName = _commandName;
        value = _value;
}

class HT12E {
    public:
    HT12E(int pin);         // this is the constructor
    int read();             // this is the main method
 
    private:
    byte _pin;              // this is Arduino input pin
    unsigned int _data;     // this is data

    unsigned int _datas[3];
    byte _tries;            // this is how many times Arduino could find valid HT12E words
    unsigned long _dur;     // pulse duration
};

HT12E::HT12E(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
    _data = 0;
}

int HT12E::read() {
    byte ctr;            // for general error handling
    _tries = 0;
    do {
        /* look for HT12E basic word's pilot stream */
        for(ctr = 0; ctr < 13; ++ctr) {
        while (digitalRead(_pin) == LOW) {                // wait for the signal to go HIGH
            delay(0);
        }
        _dur = pulseIn(_pin, LOW);

        if(_dur > 9000 && _dur < 14000) break;          // 36x(clock tick interval)
        delay(0);
        }

        /* if error, skip everything */
        if(ctr == 13) {
        _tries = 4;
        break;
        }

        /* now wait until sync bit is gone */
        for(ctr = 0; ctr < 8; ++ctr) {
        if(digitalRead(_pin) == LOW) break;
        delayMicroseconds(80);
        }

        /* if error, skip everything */
        if(ctr == 8) {
        _tries = 5;
        break;
        }

        /* let's get the address+data bits now */
        for(_datas[_tries] = 0, ctr = 0; ctr < 12; ++ctr) {
        _dur = pulseIn(_pin, HIGH);
        if(_dur > 250 && _dur < 500) {          // if pulse width is between 1/4000 and 1/3000 secs
            _datas[_tries] = (_datas[_tries] << 1) + 1;             // attach a *1* to the rightmost end of the buffer
        } else if(_dur > 500 && _dur < 900) {   // if pulse width is between 2/4000 and 2/3000 secs
        _datas[_tries] = (_datas[_tries] << 1);                  // attach a *0* to the rightmost end of the buffer
        } else {
        /* force loop termination */
            _datas[_tries] = 0;
            break;
        }
        delay(0);
        }
        ++_tries;
        delay(0);
    } while(_tries < 3);

    if(_tries > 3) { // error handling
        switch(_tries) {
        case 4: return 0xffff;
        case 5: return 0xfffe;
        case 6: return 0xfffd;
        }
    }

    if (~(_datas[0] ^ _datas[1]) & ~(_datas[1] ^ _datas[2]))
        return _datas[0];
    else
        return 0xfffc;
}


class FanReceiverBinarySensor : public Component {
    public:

    BinarySensor *fan_light_button  = new BinarySensor();
    BinarySensor *fan_low_button    = new BinarySensor();
    BinarySensor *fan_med_button    = new BinarySensor();
    BinarySensor *fan_high_button   = new BinarySensor();
    BinarySensor *fan_off_button    = new BinarySensor();

    Mapping* mappings[5];

    HT12E* remote;

    void handleMapping(Mapping* m) {
        if (m->state == m->previousState)
            return;
        m->sensor->publish_state(m->state);
    }

    void setup() override {

        remote = new HT12E(RECEIVER_PIN);

        mappings[0] = new Mapping(fan_light_button, "light_dimmer", 510);
        mappings[1] = new Mapping(fan_low_button, "fan-low", 503);
        mappings[2] = new Mapping(fan_med_button, "fan-med", 495);
        mappings[3] = new Mapping(fan_high_button, "fan-hig", 479);
        mappings[4] = new Mapping(fan_off_button, "fan-off", 509);

    }

    void loop() override {
        unsigned int data;
        data = remote->read();

        if (data <= 0xFFF0) {
            delay(0);
            ESP_LOGD("custom", "Value: %d", data);
        }
        
        for (int i = 0; i < (sizeof(mappings) / sizeof(mappings[0])); i++) {
            mappings[i]->previousState = mappings[i]->state;

            if (data == mappings[i]->value) {
            if (mappings[i]->onCount < ON_PERSISTENCE)
                mappings[i]->onCount++;
            mappings[i]->offCount = 0;
            } else {
            mappings[i]->onCount = 0;
            if (mappings[i]->offCount < OFF_PERSISTENCE)
                mappings[i]->offCount++;
            }
            delay(0);

            if (mappings[i]->onCount == ON_PERSISTENCE)
            mappings[i]->state = true;
            else if (mappings[i]->offCount == OFF_PERSISTENCE)
            mappings[i]->state = false;

            delay(0);
            handleMapping(mappings[i]);
        }
    }

};