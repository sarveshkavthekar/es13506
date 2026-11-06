#include "es13506.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define ES13506_ADDR       0x40
#define ES13506_I2C_FREQ   50000

static const char *TAG = "ES13506";

static i2c_master_dev_handle_t es_dev = NULL;
static i2c_master_bus_handle_t es_bus = NULL;

esp_err_t es13506_init(i2c_port_num_t i2c_port, int sda_gpio, int scl_gpio)
{
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = i2c_port,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err = i2c_new_master_bus(&bus_cfg, &es_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus initialization failed");
        return err;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ES13506_ADDR,
        .scl_speed_hz = ES13506_I2C_FREQ,
    };

    err = i2c_master_bus_add_device(es_bus, &dev_cfg, &es_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Sensor device initialization failed");
        i2c_del_master_bus(es_bus);
        es_bus = NULL;
        return err;
    }

    ESP_LOGI(TAG, "ES-13506 driver bound to device node");
    return ESP_OK;
}

esp_err_t es13506_read(float *temperature, float *humidity)
{
    if (temperature == NULL || humidity == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (es_dev == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t cmd;
    uint8_t data[3];
    uint16_t raw;
    esp_err_t err;

    /* ---------------- TEMPERATURE ---------------- */
    cmd = 0xF3;
    err = i2c_master_transmit(es_dev, &cmd, 1, 1000);
    if (err != ESP_OK) return err;

    vTaskDelay(pdMS_TO_TICKS(85));

    err = i2c_master_receive(es_dev, data, 3, 1000);
    if (err != ESP_OK) return err;

    raw = ((uint16_t)data[0] << 8) | (data[1] & 0xFC);
    *temperature = -46.85f + 175.72f * raw / 65536.0f;

    /* ---------------- HUMIDITY ---------------- */
    cmd = 0xF5;
    err = i2c_master_transmit(es_dev, &cmd, 1, 1000);
    if (err != ESP_OK) return err;

    vTaskDelay(pdMS_TO_TICKS(30));

    err = i2c_master_receive(es_dev, data, 3, 1000);
    if (err != ESP_OK) return err;

    raw = ((uint16_t)data[0] << 8) | (data[1] & 0xFC);
    *humidity = -6.0f + 125.0f * raw / 65536.0f;

    if (*humidity < 0)   *humidity = 0;
    if (*humidity > 100) *humidity = 100;

    return ESP_OK;
}
