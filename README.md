# GrafanaCon LA 2019 IoT Workshop

This project is a demonstration of how to monitor temperature and humidity using an ESP32 Pico board and DHT11 sensor, and store that data in a TSDB so it can be graphed with Grafana.

## Presentation

https://docs.google.com/presentation/d/1_UPUbFjVLKdW6VeS-KdOSBvoTSDJDRgiEMtXPMg3ULw/edit?usp=sharing

## Hardware

- [ESP32 Pico Kit](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/get-started-pico-kit.html#)
- [DHT11 or DHT22 Sensor](https://learn.adafruit.com/dht/overview)
- Hookup wires
- USB Power supply

## Serial Port Driver

You may need to manually install the driver for the USB to serial adapter if it doesn't show up automatically when you plug in the ESP32

- https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

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

How to get the root CA cert: https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/

## Hosted Metrics

- Sign up at https://grafana.com/loki#get
- Create an API Key with MetricsPublisher role and add it to the config file
- Go to the details page for your hosted metrics Graphite instance and get the instance id
- Set that as HM_GRAPHITE_INSTANCE

## Hosted Grafana

- Log in to your instance
- Import the dashboard from https://grafana.com/dashboards/9848
- Select your hosted metrics Graphite instance when prompted
