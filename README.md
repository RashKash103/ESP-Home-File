# Fan Remote Receiver

This provides an interface between a physical fan remote (like one of [these](https://i.imgur.com/g9jsem4.jpg)) and a [Home Assistant](https://www.home-assistant.io/) installation. This implementation uses [ESPHome](https://esphome.io/) to enable the communication from an ESP8266 device. For this project, I'm using the NodeMCU development board but any development board utilizing the ESP8266 or the ESP32 should work (probably with some modifications).

## Hardware Setup
Currently, the hardware setup is lackluster. I will improve it if anyone asks for improvements.
- Extract the daughter board and antenna from a fan receiver (I used [this](https://www.amazon.com/gp/product/B07TCMWDDM/) one but most should work).
- Connect the antenna directly to the daughter board.
- Connect the `V` and `G` labelled pins to the `3.3v` and `GND` pins of the development board, respectively.
- Connect the center data pin of the daughter board to a digital input pin of the development board. I used pin `5`. Change the `#define RECEIVER_PIN 5` line if you use another pin.
- Set up the software!

## Software Setup

- Follow the [Home Assistant Getting Started](https://www.home-assistant.io/getting-started/) tutorial to get that setup.
- Follow the [ESPHome tutorial for Home Assistant](https://esphome.io/guides/getting_started_hassio.html) to get ESPHome hooked up. I recommend installing the **SSH & Web Terminal** and **File editor** add-ons from the add-on store as well to make your life easier.
- After setting that up, create a node according to the ESPHome tutorial. Then copy the contents of `fan_remote_receiver.yaml` into the generated config file. Be sure not to override your `esphome.name`, `esphome.platform`, and `esphome.board` if it differs from the `yaml` provided. Copy `fan_receiver.h` to the `config/esphome/` directory. This can be done easily using the **File editor** add-on mentioned before.
- In ESPHome, generate the binary file by clicking on the three dots to the right of the node card, clicking **Compile**, and then cicking **Download Binary**. I recommend using the [ESPHome Flasher](https://esphome.io/guides/faq.html#esphome-flasher) for the very first upload to your board. After that, you can use ESPHome's **Upload** functionality to update the board OTA. Pretty cool!
- Once uploaded, the board will automatically connect to the Home Assistant installation and you should see the buttons in the Overview tab of Home Assistant.

## Link Remote

The remote I used for this had the DIP switches in the `1100` position. That means, the switches labeled `1` and `2` were in the `ON` position and the switches labeled `3` and `4` were in the `OFF` position. If you can use that combination as well, the included `fan_receiver.h` file will work for you as is.

If you want to use any code other than `1100`, then after uploading the software to the board, open up the `LOGS` on the ESPHome panel. Once the log viewer opens up, press any button on your remote and it will show `Value: XXX` where `XXX` is the number you need to note down for the pressed button. Then, modify the lines show below in `fan_receiver.h` to include the correct corresponding codes.

```arduino
    void setup() override {

        remote = new HT12E(RECEIVER_PIN);

        mappings[0] = new Mapping(fan_light_button, "light_dimmer", 510);
        mappings[1] = new Mapping(fan_low_button, "fan-low", 503);
        mappings[2] = new Mapping(fan_med_button, "fan-med", 495);
        mappings[3] = new Mapping(fan_high_button, "fan-hig", 479);
        mappings[4] = new Mapping(fan_off_button, "fan-off", 509);

    }
```

## Credits

This would not have been possible were it not for the HT12E library created by user [neuron_upheaval](https://forum.arduino.cc/index.php?action=profile;u=5600) on the Arduino forums. Link to the post [here](https://forum.arduino.cc/index.php?topic=54788.0).

## Contributing
Feel free to submit a pull request!

## License
[MIT](https://choosealicense.com/licenses/mit/)