/**
 * gpiobug - teste do bug do latch do gpio no RP2350
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint BTN_PD_INTERNO = 13;
const uint BTN_PD_10k     = 14;
const uint BTN_PD_4k7     = 15;
const uint btn[3] = {BTN_PD_INTERNO, BTN_PD_10k, BTN_PD_4k7};

int main() {
  // Inicia stdio
  stdio_init_all();
  #ifdef LIB_PICO_STDIO_USB
  while (!stdio_usb_connected()) {
      sleep_ms(100);
  }
  #endif

  // Se identifica
  printf("GPIOBUG v1.00\n");

  #if PICO_RP2040
    #pragma message("Running on RP2040 - ARM Cortex-M0+")
    printf("Running on RP2040 - ARM Cortex-M0+\n\n");
  #elif PICO_RP2350
    #if PICO_RISCV
      #pragma message("Running on RP2350 - RISC-V Hazard3")
      printf("Running on RP2350 - RISC-V Hazard3\n\n");
    #else
      #pragma message("Running on RP2350 - ARM Cortex-M3")
        printf("Running on RP2350 - ARM Cortex-M3\n\n");
    #endif
  #else
    #pragma message("Running on ???")
    printf("Running on ???\n");
  #endif

  // Inicia os pinos
  gpio_init(BTN_PD_INTERNO);
  gpio_set_dir(BTN_PD_INTERNO, GPIO_IN);
  gpio_pull_down(BTN_PD_INTERNO);

  gpio_init(BTN_PD_10k);
  gpio_set_dir(BTN_PD_10k, GPIO_IN);
  gpio_disable_pulls(BTN_PD_10k);

  gpio_init(BTN_PD_4k7);
  gpio_set_dir(BTN_PD_4k7, GPIO_IN);
  gpio_disable_pulls(BTN_PD_4k7);

  bool a[3], b[3];

  for (int i = 0; i < 3; i++) {
    a[i] = !gpio_get(btn[i]);
  }

  while(1) {
    bool mudou = false;
    for (int i = 0; i < 3; i++) {
      b[i] = gpio_get(btn[i]);
      if (b[i] != a[i]) {
        mudou = true;
      }
    }
    if (mudou) {
      for (int i = 0; i < 3; i++) {
        a[i] = b[i];
      }
      printf ("%d %d %d\n", a[0], a[1], a[2]);
      sleep_ms(100);  // "debounce"
    } else {
      sleep_ms(10);
    }
  }
}
