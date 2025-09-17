#include <avr/io.h>

uint16_t readVdd() {
  // Use VDD as ADC reference
  ADC0.CTRLC = ADC_PRESC_DIV4_gc | ADC_REFSEL_VDDREF_gc;

  // Select input channel: internal 1.1V reference
  ADC0.MUXPOS = ADC_MUXPOS_INTREF_gc;

  // Set reference to 1.1V explicitly
  VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc;

  // Enable ADC
  ADC0.CTRLA = ADC_ENABLE_bm;

  // Start conversion
  ADC0.COMMAND = ADC_STCONV_bm;
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
    ;

  uint16_t result = ADC0.RES;
  ADC0.INTFLAGS = ADC_RESRDY_bm;

  // VDD = (1.1V * 1023) / ADCresult
  if (result == 0) return 0;
  uint32_t vdd_mv = (1100UL * 1023UL) / result;

  return (uint16_t)vdd_mv;
}

bool battCheck() {
  uint16_t vdd = readVdd();
  if (vdd <= BAT_VOL_MIN) {
   return false;
  }else{
    return true;
  }
}