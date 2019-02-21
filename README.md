# GrafanaCon LA 2019 IoT Workshop

This project is a demonstration of how to monitor temperature and humidity using an ESP32 Pico board and DHT11 sensor, and store that data in a TSDB so it can be graphed with Grafana.

## Hardware

- [ESP32 Pico Kit](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/get-started-pico-kit.html#)
- [DHT11 or DHT22 Sensor](https://learn.adafruit.com/dht/overview)
- Hookup wires
- USB Power supply

## Arduino IDE Setup

- [Arduino IDE](https://www.arduino.cc/en/Main/Software)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)

### Required Libraries

- [Adafruit DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
- [Adafruit Unified Sensor Library](https://github.com/adafruit/Adafruit_Sensor)
- [NTPClient Library](https://github.com/arduino-libraries/NTPClient)

## Download project into your Arduino sketchbook folder

- `git clone https://github.com/DanCech/IoTWorkshop.git`, or
- [Download Zip](https://github.com/DanCech/IoTWorkshop/archive/master.zip) & extract it

## Config file

- Copy `config.h.example` to `config.h`
- Fill in WiFi connection details
- Set timezone offset for formatted dates
- Set a unique name for this sensor
- Adjust reporting interval if required

### Hosted Metrics

- Sign up at https://grafana.com/loki#get

