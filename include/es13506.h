#ifndef ES13506_H
#define ES13506_H

#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the ES-13506 sensor.
 *
 * @param i2c_port I2C controller to use (e.g., 0).
 * @param sda_gpio SDA GPIO number.
 * @param scl_gpio SCL GPIO number.
 *
 * @return ESP_OK on success.
 */
esp_err_t es13506_init(
    i2c_port_num_t i2c_port,
    int sda_gpio,
    int scl_gpio
);

/**
 * @brief Read temperature and relative humidity.
 *
 * @param temperature Temperature output in Celsius.
 * @param humidity Relative humidity output in %RH.
 *
 * @return ESP_OK on success.
 */
esp_err_t es13506_read(
    float *temperature,
    float *humidity
);

#ifdef __cplusplus
}
#endif

#endif // ES13506_H
