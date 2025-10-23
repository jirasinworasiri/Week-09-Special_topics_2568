#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define LED1_GPIO GPIO_NUM_2
#define LED2_GPIO GPIO_NUM_4
#define LED3_GPIO GPIO_NUM_5

#define LEDC_MODE       LEDC_HIGH_SPEED_MODE
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_CHANNEL1   LEDC_CHANNEL_0
#define LEDC_CHANNEL2   LEDC_CHANNEL_1
#define LEDC_CHANNEL3   LEDC_CHANNEL_2
#define LEDC_DUTY_RES   LEDC_TIMER_10_BIT // 10-bit (0-1023)
#define LEDC_FREQUENCY  5000              // 5 kHz

#define FADE_TIME       500  // เวลา fade (ms)

static const char *TAG = "LED_BREATH";

/**
 * @brief กำหนดค่าเริ่มต้น PWM สำหรับ LED 3 ดวง
 */
void led_init(void) {
    // ตั้งค่า timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // ตั้งค่า channel
    ledc_channel_config_t ledc_channel[3] = {
        {.gpio_num = LED1_GPIO, .speed_mode = LEDC_MODE, .channel = LEDC_CHANNEL1, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER, .duty = 0, .hpoint = 0},
        {.gpio_num = LED2_GPIO, .speed_mode = LEDC_MODE, .channel = LEDC_CHANNEL2, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER, .duty = 0, .hpoint = 0},
        {.gpio_num = LED3_GPIO, .speed_mode = LEDC_MODE, .channel = LEDC_CHANNEL3, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER, .duty = 0, .hpoint = 0},
    };

    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[i]));
    }

    // ⭐ ติดตั้ง fade service
    ESP_ERROR_CHECK(ledc_fade_func_install(0));

    ESP_LOGI(TAG, "LED PWM initialization completed");
}

/**
 * @brief ฟังก์ชัน fade LED
 */
void fade_led(int channel, int duty, int time_ms) {
    ledc_set_fade_time_and_start(LEDC_MODE, channel, duty, time_ms, LEDC_FADE_WAIT_DONE);
}

/**
 * @brief Pattern 1: Knight Rider (Breathing)
 */
void knight_rider_task(void *pvParameters) {
    while (1) {
        // วิ่งไปทางขวา
        fade_led(LEDC_CHANNEL1, 1023, FADE_TIME);
        fade_led(LEDC_CHANNEL1, 0, FADE_TIME);

        fade_led(LEDC_CHANNEL2, 1023, FADE_TIME);
        fade_led(LEDC_CHANNEL2, 0, FADE_TIME);

        fade_led(LEDC_CHANNEL3, 1023, FADE_TIME);
        fade_led(LEDC_CHANNEL3, 0, FADE_TIME);

        // วิ่งกลับทางซ้าย
        fade_led(LEDC_CHANNEL2, 1023, FADE_TIME);
        fade_led(LEDC_CHANNEL2, 0, FADE_TIME);
    }
}

/**
 * @brief Pattern 2: Binary Counter (Breathing)
 */
void binary_counter_task(void *pvParameters) {
    while (1) {
        for (int i = 0; i < 8; i++) {
            int b1 = (i >> 0) & 1;
            int b2 = (i >> 1) & 1;
            int b3 = (i >> 2) & 1;

            fade_led(LEDC_CHANNEL1, b1 ? 1023 : 0, FADE_TIME);
            fade_led(LEDC_CHANNEL2, b2 ? 1023 : 0, FADE_TIME);
            fade_led(LEDC_CHANNEL3, b3 ? 1023 : 0, FADE_TIME);

            ESP_LOGI(TAG, "Binary: %d%d%d", b3, b2, b1);
            vTaskDelay(pdMS_TO_TICKS(FADE_TIME));
        }
    }
}

/**
 * @brief Pattern 3: Random Blinking (Breathing)
 */
void random_blink_task(void *pvParameters) {
    srand((unsigned) time(NULL));
    while (1) {
        int l1 = rand() % 2;
        int l2 = rand() % 2;
        int l3 = rand() % 2;

        fade_led(LEDC_CHANNEL1, l1 ? 1023 : 0, FADE_TIME);
        fade_led(LEDC_CHANNEL2, l2 ? 1023 : 0, FADE_TIME);
        fade_led(LEDC_CHANNEL3, l3 ? 1023 : 0, FADE_TIME);

        ESP_LOGI(TAG, "Random: %d %d %d", l1, l2, l3);
        vTaskDelay(pdMS_TO_TICKS(FADE_TIME));
    }
}

/**
 * @brief main()
 */
void app_main(void) {
    led_init();

    // ✅ เลือก pattern ที่ต้องการ (comment/uncomment ได้ทีละอัน)
    //xTaskCreate(knight_rider_task, "knight_rider", 4096, NULL, 5, NULL);
    //xTaskCreate(binary_counter_task, "binary_counter", 4096, NULL, 5, NULL);
    xTaskCreate(random_blink_task, "random_blink", 4096, NULL, 5, NULL);
}
