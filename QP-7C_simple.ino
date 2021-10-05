#include "si5351.h"

Si5351 si5351;
long int freq = 7010000; // freqency in Hz
   
void setup(void)
{
  //si5351の準備 ここから  
  int32_t cal_factor = 32392;
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, cal_factor); 
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
 
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(freq*100ULL, SI5351_CLK0);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK0, 0);
  //si5351の準備 ここまで  

  si5351.output_enable(SI5351_CLK0, 1);   //CLK0 on
}

void loop(void)
{
}
