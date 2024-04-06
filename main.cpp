#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// board constants 
int PICO_BOARD_FREQUENCY = 125'000'000;
int NOCTUA_PWM_FREQUENCY = 25'000;

// code constants
int NOCTUA_PWM_PIN = 16;
int LED_ONBAORD = 25;

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

  void set_duty_cycle(float duty_ratio_high) {
    pwm_set_gpio_level(pwm_pin, int(float(pwm_wrap_tics) * duty_ratio_high));
  }

};

hardware_pwm noctua_fan(NOCTUA_PWM_PIN, NOCTUA_PWM_FREQUENCY);

// configure the board
void init() {
    // use the onboard led to make sure our board is working
    gpio_init(LED_ONBAORD);
    gpio_set_dir(LED_ONBAORD, GPIO_OUT);

    noctua_fan.init();
}

int main() {
    stdio_init_all();
    init();

    while (true) {
        gpio_put(LED_ONBAORD, 1);
        sleep_ms(1000);
        gpio_put(LED_ONBAORD, 0);
        sleep_ms(1000);

        printf("Hello, world!\n");
    }

}

