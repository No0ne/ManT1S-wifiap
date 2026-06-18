/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 No0ne (https://github.com/No0ne)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_private/wifi.h"
#include "esp_log.h"
#include "esp_eth.h"
#include "esp_eth_phy_lan867x.h"

#define WIFI_AP_SSID "ManT1S-wifiap"
#define WIFI_AP_PASSWORD "testing123"
#define WIFI_AP_COUNTRY "AT"
#define WIFI_AP_CHANNEL 13

esp_eth_handle_t handle = NULL;

static esp_err_t pkt_eth2wifi(esp_eth_handle_t eth, uint8_t *buffer, uint32_t len, void *priv) {
  esp_wifi_internal_tx(WIFI_IF_AP, buffer, len);
  free(buffer);
  return ESP_OK;
}

static esp_err_t pkt_wifi2eth(void *buffer, uint16_t len, void *eb) {
  esp_eth_transmit(handle, buffer, len);
  esp_wifi_internal_free_rx_buffer(eb);
  return ESP_OK;
}

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_country_code(WIFI_AP_COUNTRY, true));
  ESP_ERROR_CHECK(esp_wifi_internal_reg_rxcb(WIFI_IF_AP, pkt_wifi2eth));

  wifi_config_t ap = {
    .ap = {
      .ssid = WIFI_AP_SSID,
      .ssid_len = strlen(WIFI_AP_SSID),
      .channel = WIFI_AP_CHANNEL,
      .password = WIFI_AP_PASSWORD,
      .authmode = WIFI_AUTH_WPA2_PSK,
      .max_connection = 10,
    },
  };

  if(strlen(WIFI_AP_PASSWORD) == 0) {
    ap.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));

  eth_mac_config_t mac_cfg = ETH_MAC_DEFAULT_CONFIG();
  eth_esp32_emac_config_t esp32_cfg = ETH_ESP32_EMAC_DEFAULT_CONFIG();
  esp32_cfg.smi_gpio.mdc_num = 8;
  esp32_cfg.smi_gpio.mdio_num = 7;
  esp32_cfg.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
  esp32_cfg.clock_config.rmii.clock_gpio = EMAC_CLK_IN_GPIO;
  esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_cfg, &mac_cfg);

  eth_phy_config_t phy_cfg = ETH_PHY_DEFAULT_CONFIG();
  phy_cfg.phy_addr = -1;
  phy_cfg.reset_gpio_num = -1;
  esp_eth_phy_t *phy = esp_eth_phy_new_lan867x(&phy_cfg);

  esp_eth_config_t eth_cfg = ETH_DEFAULT_CONFIG(mac, phy);
  ESP_ERROR_CHECK(esp_eth_driver_install(&eth_cfg, &handle));

  bool promiscuous = true;
  ESP_ERROR_CHECK(esp_eth_ioctl(handle, ETH_CMD_S_PROMISCUOUS, &promiscuous));
  ESP_ERROR_CHECK(esp_eth_update_input_path(handle, pkt_eth2wifi, NULL));

  ESP_ERROR_CHECK(esp_eth_start(handle));
  ESP_ERROR_CHECK(esp_wifi_start());
}
