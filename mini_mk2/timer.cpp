#include "stm32f103xb.h"
#include "timer.h"
#include "stm32f1xx.h"


static volatile bool interrupt_call = false;

bool get_interrupt_call() {
  return interrupt_call;
}

void set_interrupt_call(bool state) {
  interrupt_call = state;
}

void (*callback)();

void setup_timer() {
  TIM3->PSC = 999; // prescaler value for a timer frequency of 72 kHz
  TIM3->ARR = 7199; // auto-reload value for a 1 ms overflow
  
  TIM3->CR1 |= TIM_CR1_CEN; // Enable timer

  // Disable slave mode (use internal clock)
  TIM3->SMCR |= TIM_SMCR_SMS;

  TIM3->DIER |= TIM_DIER_UIE; // Enable update interrupt
  
  NVIC_EnableIRQ(TIM3_IRQn); // Enable interrupt in NVIC

  interrupt_call = true;
}

extern "C" __attribute__((weak)) void TIM3_IRQHandler(void) {
    
    interrupt_call = true;
    // Check if the interrupt was caused by the update event (overflow)
    if (TIM3->SR & TIM_SR_UIF) {
        // Clear the interrupt flag
        TIM3->SR &= ~TIM_SR_UIF;

        // Set interrupt_call flag to true
        interrupt_call = true;

        // Add your interrupt handling code here
        // For example, toggle an LED or increment a counter
    }
}
