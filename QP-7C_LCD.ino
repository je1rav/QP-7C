#include "LCD_ST7032.h"
#include "si5351.h"

LCD_ST7032 lcd;
Si5351 si5351;
long int freq = 7010000; // 初期周波数(Hz)
int cursol = 5;  //初期周波数変化幅を1kHzにする（表示器の５桁目）
long int freqstep = 1000;  //初期周波数変化幅を1kHzにする

void setup(void)
{
  //si5351の準備 ここから-----  
  int32_t cal_factor = -20000;     //モジュールに依存します（要調整）。
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0); 
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(freq*100ULL, SI5351_CLK0);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK0, 0);
  //si5351の準備 -----ここまで  

  //ST7032(AQM0802A-RN-GBW, 秋月電子)の準備 ここから----- 
  lcd.begin();
  lcd.setcontrast(20);
  lcd.setCursor(0,0);
  lcd.print("QP-7C   ");
  //ST7032(AQM0802A-RN-GBW)の準備 -----ここまで 

  //デジタル入力ピンの準備 ここから----- 
  pinMode(8, INPUT_PULLUP); //SW1 (step up)
  pinMode(9, INPUT_PULLUP); //SW2 (freq. up)
  pinMode(10, INPUT_PULLUP); //SW3 (freq. down)
  //デジタル入力ピンの準備 -----ここまで 
  
  si5351.output_enable(SI5351_CLK0, 1);   //CLK0 出力ON
  lcd_disp(freq, cursol);
}

void loop(void)
{
  long int freqset; 
  
  // 変化させる周波数の桁の変更
  if (digitalRead(8)==LOW){
    cursol++;
    freqstep = freqstep / 10;
    if (cursol>8) {   //端まできたらもどす
      cursol=2; // 左からのcursol位置、最大step幅1MHzに対応 (10MHzにしたければ2を1に)
      freqstep = 1000000; // 最大step幅 (cursolの値に合わせて変更)
    }
  }
  
  // 周波数アップ
  if (digitalRead(9)==LOW){
    freqset = freq + freqstep;
    freq=freq_in_band(freq, freqset); // オフバンド防止関数へ
    si5351.set_freq(freq*100ULL, SI5351_CLK0);
  }
  
  // 周波数ダウン
  if (digitalRead(10)==LOW){
    freqset = freq - freqstep;
    freq=freq_in_band(freq, freqset); // オフバンド防止関数へ
    si5351.set_freq(freq*100ULL, SI5351_CLK0);
  }
  lcd_disp(freq, cursol);
}

//LCDでの周波数表示用の関数、変化させる周波数の桁を点滅
void lcd_disp(long int frequency, int cursolposition)
{
  lcd.setCursor(1,0);
  lcd.print("        ");
  
  String freqString =  String(frequency, DEC);
  lcd.setCursor(1,8-freqString.length());
  lcd.print(freqString);
  delay(100);
  lcd.setCursor(1,cursolposition-1);
  lcd.print("_");
  delay(40);
}

//送信周波数がオフバンドしないように強制的に周波数を制限する関数
long int freq_in_band(long int frequency, long int freqset)
{
  long int freq_in_band;
  if (freqset < 135700) {
    freq_in_band=135700;
    return freq_in_band;
  }
  else if (freqset > 135700 && freqset < 472000) {
    if (freqset>frequency) freq_in_band=472000;
    else freq_in_band=135800;
    return freq_in_band;
  }
  else if (freqset > 479000 && freqset < 1800000) {
    if (freqset>frequency) freq_in_band=1800000;
    else freq_in_band=479000;
    return freq_in_band;
  }
  else if (freqset > 1875000 && freqset < 1907500) {
    if (freqset>frequency) freq_in_band=1907500;
    else freq_in_band=1875000;
    return freq_in_band;
  }
  else if (freqset > 1912500 && freqset < 3500000) {
    if (freqset>frequency) freq_in_band=3500000;
    else freq_in_band=1912500;
    return freq_in_band;
  }
  else if (freqset > 3580000 && freqset < 3662000) {
    if (freqset>frequency) freq_in_band=3662000;
    else freq_in_band=3580000;
    return freq_in_band;
  }
  else if (freqset > 3687000 && freqset < 3716000) {
    if (freqset>frequency) freq_in_band=3716000;
    else freq_in_band=3687000;
    return freq_in_band;
  }
  else if (freqset > 3770000 && freqset < 3791000) {
    if (freqset>frequency) freq_in_band=3791000;
    else freq_in_band=3770000;
    return freq_in_band;
  }
  else if (freqset > 3805000 && freqset < 7000000) {
    if (freqset>frequency) freq_in_band=7000000;
    else freq_in_band=3805000;
    return freq_in_band;
  }
  else if (freqset > 7200000 && freqset < 10100000) {
    if (freqset>frequency) freq_in_band=10100000;
    else freq_in_band=7200000;
    return freq_in_band;
  }
  else if (freqset > 10150000 && freqset < 14000000) {
    if (freqset>frequency) freq_in_band=14000000;
    else freq_in_band=10150000;
    return freq_in_band;
  }
  else if (freqset > 14350000 && freqset < 18068000) {
    if (freqset>frequency) freq_in_band=18068000;
    else freq_in_band=14350000;
    return freq_in_band;
  }
  else if (freqset > 18168000 && freqset < 21000000) {
    if (freqset>frequency) freq_in_band=21000000;
    else freq_in_band=18168000;
    return freq_in_band;
  }
  else if (freqset > 21450000 && freqset < 24890000) {
    if (freqset>frequency) freq_in_band=24890000;
    else freq_in_band=21450000;
    return freq_in_band;
  }
  else if (freqset > 24990000 && freqset < 28000000) {
    if (freqset>frequency) freq_in_band=28000000;
    else freq_in_band=24990000;
    return freq_in_band;
  }
  else if (freqset > 29700000 && freqset < 50000000) {
    if (freqset>frequency) freq_in_band=50000000;
    else freq_in_band=29700000;
    return freq_in_band;
  }
  else if (freqset > 54000000) {
    freq_in_band=54000000;
    return freq_in_band;
  }
  else {
    freq_in_band=freqset;
    return freq_in_band;
  }
}
