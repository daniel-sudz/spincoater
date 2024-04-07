#include <stdio.h>
#include <string>
#include <iostream>

#include "pico/stdlib.h"
#include "hardware/pwm.h"

// board constants 
int PICO_BOARD_FREQUENCY = 125'000'000;
int NOCTUA_PWM_FREQUENCY = 25'000;

// code constants
int NOCTUA_PWM_PIN = 16;
int NOCTUA_TOC_PIN = 17;
int LED_ONBAORD = 25;

struct Utils {
    static uint64_t get_time_us() {
        return to_us_since_boot(get_absolute_time());
    }
    static uint64_t get_time_ms() {
        return to_ms_since_boot(get_absolute_time());
    }
};

struct periodic_logger {
    uint64_t last_reading_time_us;
    uint64_t throttle_ms;
    uint64_t last_log_time_ms;
    std::string logger_name;
    std::string queued_logger_msg; 

    periodic_logger(std::string logger_name, uint64_t throttle_ms) {
        this->logger_name = logger_name;
        this->throttle_ms = throttle_ms;
        this->last_log_time_ms = Utils::get_time_ms();
    }

    void log_msg(std::string msg) {
        queued_logger_msg = msg;
    }

    void loop() {
        uint64_t current_time = Utils::get_time_ms();
        if((current_time - last_log_time_ms) >  throttle_ms) {
            if(queued_logger_msg != "") {
                std::cout<<"[LOGGER: "<<logger_name<<"]: "<<queued_logger_msg<<std::endl;
                queued_logger_msg = "";
            }
            last_log_time_ms = current_time;
        }
    }

};

struct hardware_pwm {
    int pwm_pin;
    int pwm_freq;
    int pwm_wrap_tics;

    hardware_pwm(int pwm_pin, int pwm_freq) {
        this->pwm_pin = pwm_pin;
        this->pwm_freq = pwm_freq;
        this->pwm_wrap_tics =  PICO_BOARD_FREQUENCY / pwm_freq;
    }

    void init() {
        // use the hardware pwm controllers
        // https://github.com/raspberrypi/pico-examples/blob/master/pwm/hello_pwm/hello_pwm.c
        gpio_init(pwm_pin);
        gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

        uint pwm_slice_num = pwm_gpio_to_slice_num(pwm_pin);
        pwm_set_wrap(pwm_slice_num, pwm_wrap_tics);
        pwm_set_enabled(pwm_slice_num, true);
        pwm_set_gpio_level(pwm_pin, pwm_wrap_tics / 2);
    }

    void set_duty_cycle_low_ratio(float duty_ratio_low) {
        pwm_set_gpio_level(pwm_pin, int(float(pwm_wrap_tics) * duty_ratio_low));
    }
    void set_duty_cycle_high_ratio(float duty_ratio_low) {
        set_duty_cycle_low_ratio(1.0f - duty_ratio_low);
    }
};

struct tocometer {
    int tocometer_pin;
    int last_reading = 0;
    uint64_t last_reading_time_us;
    uint64_t rpm = 0;
    uint64_t debounce_us = 100;

    periodic_logger tocometer_debug = periodic_logger("TOCOMETER", 1000);

    tocometer(int tocometer_pin) {
        this->tocometer_pin = tocometer_pin;
        this->last_reading_time_us = Utils::get_time_us();
    }

    void init() {
      gpio_init(tocometer_pin);
      gpio_set_dir(tocometer_pin, GPIO_IN);
    }

    void loop() {
        uint64_t cur_time = Utils::get_time_us();
        if(cur_time - last_reading_time_us > debounce_us) {
            if(gpio_get(tocometer_pin) != last_reading) {
                // two tocometer pos edge tics per revolution
                rpm = ((cur_time - last_reading_time_us) * 60) / 4;
                last_reading_time_us = cur_time;
            }
        }

        tocometer_debug.log_msg("RPM: " + std::to_string(rpm));
    }

};

struct led_debug_blink {
    int led_pin;
    bool led_state = 0;
    uint64_t last_time_blink_ms;
    uint64_t blink_on_duration_ms = 1000;

    led_debug_blink(int led_pin) {
        this->led_pin = led_pin; 
        last_time_blink_ms = Utils::get_time_ms();
    }

    void init() {
        gpio_init(led_pin);
        gpio_set_dir(led_pin, GPIO_OUT);
    }

    void loop() {
        uint64_t current_time = Utils::get_time_ms();
        if((current_time - last_time_blink_ms) > blink_on_duration_ms) {
            gpio_put(led_pin, !led_state);
            led_state = !led_state;
            last_time_blink_ms = current_time;
        }
    }

};

hardware_pwm noctua_fan(NOCTUA_PWM_PIN, NOCTUA_PWM_FREQUENCY);
tocometer noctual_toc(NOCTUA_TOC_PIN);
led_debug_blink onboard_led(LED_ONBAORD);

periodic_logger heartbeat_msg("HEARTBEAT", 1000);

// configure the board
void init() {
    noctua_fan.init();
    noctual_toc.init();
    onboard_led.init();
}

int main() {
    stdio_init_all();
    init();

    while (true) {
        noctua_fan.set_duty_cycle_low_ratio(0.5);
        noctual_toc.loop();
        onboard_led.loop();

        heartbeat_msg.log_msg("Alive!");
    }

}

