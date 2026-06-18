# ManT1S — Wi-Fi Access Point bridged to 10BASE-T1S

Firmware for the [Silicognition ManT1S](https://mant1s.net) board that turns it into a
**transparent Wi-Fi access point bridged onto a 10BASE-T1S single-pair Ethernet segment**.
Wi-Fi clients and T1S nodes end up in one Ethernet broadcast domain.

# Build
Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation), then:

```sh
idf.py set-target esp32
idf.py build
idf.py flash
```
