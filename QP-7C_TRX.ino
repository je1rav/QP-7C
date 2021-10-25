#include "si5351.h"
#include "Wire.h"
//#include "avr/io.h"
#include "LCD_ST7032.h"

LCD_ST7032 lcd;
Si5351 si5351;

long int freq;   // frequency (Hz)
long int freqstep = 1000;  //initial frequency step (Hz)
int cursol = 5;            // =8-log(freqstep)

int mode; //0=CW mode (single key), 2=Digital mode
int cw_tone = 600;  // in Hz (USB:+, LSB:-)
int TxStatus = 0; //0=RX, 1=TX
int statuschangeflag= 0; //1=staus change
int key;
int freqchangeflag=0;

void setup(void)
{
  Serial.begin(115200); 
  Serial.setTimeout(4);
  
  //si5351 initialization-----  
  int32_t cal_factor = -21800;
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0); 
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(freq*100ULL, SI5351_CLK0);  //for TX
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.set_freq(freq*100ULL/2, SI5351_CLK1);  //for RX
//  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK1, 0);
  //si5351 ----- 

  //ST7032(AQM0802A-RN-GBW) initialization----- 
  lcd.begin();
  lcd.setcontrast(20);
  lcd.blink();  //cursol blinking
  lcd.setCursor(0,0);
  lcd.print("QP-7C   ");
  //ST7032 -----

  //Digital pins ----- 
  pinMode(2, INPUT_PULLUP); //Key
  pinMode(7, INPUT); //PD7 = AN1 = HiZ, PD6 = AN0 = 0
  pinMode(8, INPUT_PULLUP); //SW1 (freq. step down)
  pinMode(9, INPUT_PULLUP); //SW2 (freq. up)
  pinMode(10, INPUT_PULLUP); //SW3 (freq. down)
  pinMode(11, OUTPUT); //High=RX, LOW=TX (for RX switch)
  pinMode(12, OUTPUT); //High=RX, LOW=TX (for Drive switch)
  pinMode(13, OUTPUT); //High=TX, LOW=RX (for Drive switch (NPN open-collector or N-FET open-drain))
  //Digital pins ----- 
  
  //Timer setting for audio frequency analysis----- 
  TCCR1A = 0x00;
  TCCR1B = 0x01; // Timer1 Timer 16 MHz
  TCCR1B = 0x81; // Timer1 Input Capture Noise Canceler
  ACSR |= (1<<ACIC);  // Analog Comparator Capture Input
  //Timer setting -----  

  //Mode check -----  
  key=digitalRead(2);
  if (key==1) {
    mode = 0; //CW mode (single key)
    freq=7010000;
    lcd.setCursor(0,0);
    lcd.print("CW      ");
    lcd.setCursor(0,7);
    lcd.print("R");
    lcd_disp(freq, cursol);
    si5351.set_freq(freq*100ULL, SI5351_CLK0);   //TX freequency 
    si5351.set_freq((freq-cw_tone)*100ULL, SI5351_CLK1);   //RX freequency
    si5351.output_enable(SI5351_CLK0, 0);   //TX off
    si5351.output_enable(SI5351_CLK1, 1);   //RX on
    digitalWrite(11,1);  //RX switch on
    digitalWrite(12,1);  //RX on (TX off)
    digitalWrite(13,0);  //TX switch off
    }
  else {
    mode = 2; //Digital(FSK) mode
    freq=7041000;  //FT8 (JA local) 
    lcd.setCursor(0,0);
    lcd.print("FSK     ");
    String freqString =  String(freq, DEC);
    lcd.setCursor(1,8-freqString.length());
    lcd.print(freqString);
    si5351.set_freq(freq*100ULL, SI5351_CLK0);   //TX freequency 
    si5351.set_freq(freq*100ULL, SI5351_CLK1);   //RX freequency
    si5351.output_enable(SI5351_CLK0, 0);   //TX off
    si5351.output_enable(SI5351_CLK1, 1);   //RX on
    digitalWrite(11,1);  //RX switch on
    digitalWrite(12,1);  //RX on (TX off)
    digitalWrite(13,0);  //TX switch off
  }
}

void loop(void)
{
  if (mode == 0) cw0();
  else digital();
}

void cw0(void)
{
  long int freqset=freq;   
  // change the frequency by remote control 
    if(Serial.available() > 0) cat();
   if (freqset|=freq) {
    freq=freq_in_band(freqset, freq); // prevention of out-of-band transmission (JA)
  }
      
  // change the frequency step by SW1
  if (digitalRead(8)==LOW){
    delay(200);
    cursol++;
    freqstep = freqstep / 10;
    if (cursol>8) {
      cursol=2;
      freqstep = 1000000;
    }
    lcd_disp(freq, cursol);
  }
  
  // increase the frequency by SW2
  if (digitalRead(9)==LOW){
    delay(200);
    freqset = freq + freqstep;
    freq=freq_in_band(freq, freqset); // prevention of out-of-band transmission (JA)
    si5351.set_freq(freq*100ULL, SI5351_CLK0); //TX
    si5351.set_freq((freq-cw_tone)*100ULL, SI5351_CLK1); //RX   
    lcd_disp(freq, cursol);
  }
  
  // decrease the frequency by SW3
  if (digitalRead(10)==LOW){
    delay(200);
    freqset = freq - freqstep;
    freq=freq_in_band(freq, freqset); // prevention of out-of-band transmission (JA)
    si5351.set_freq(freq*100ULL, SI5351_CLK0); //TX
    si5351.set_freq((freq-cw_tone)*100ULL, SI5351_CLK1); //RX   
    lcd_disp(freq, cursol);
  }

  // Keying
  if (digitalRead(2)==LOW) {
    if (TxStatus==0) {
      si5351.output_enable(SI5351_CLK1, 0);   //RX off
      digitalWrite(11,0);
      digitalWrite(12,0);
      si5351.output_enable(SI5351_CLK0, 1);   //TX on
      digitalWrite(13,1);
      lcd.setCursor(0,7);
      lcd.print("T");
      TxStatus=1;
    }
  }
  else {
    if (TxStatus==1) {
      si5351.output_enable(SI5351_CLK0, 0);   //TX off
      si5351.output_enable(SI5351_CLK1, 1);   //RX on
      digitalWrite(11,1);
      digitalWrite(12,1);
      digitalWrite(13,0);
      lcd.setCursor(0,7);
      lcd.print("R");
      lcd_disp(freq, cursol);
      TxStatus=0;
    }
  }
}

void digital(void)
{
 //https://www.elektronik-labor.de/HF/FT8QRP.html
 //partly modified
 //(Using 3 cycles for timer sampling to improve the precision of frequency measurements)
 //(Against overflow in low frequency measurements)
 
 if(Serial.available() > 0) cat(); 
 if(freqchangeflag==1){
    lcd.setCursor(1,0);
    lcd.print("        ");
    String freqString =  String(freq, DEC);
    lcd.setCursor(1,8-freqString.length());
    lcd.print(freqString);
    lcd_disp(freq, cursol);
    freqchangeflag=0;
 }

 int FSK = 1;
 int FSKtx = 0;

 while (FSK>0){
 int Nsignal = 10;
 int Ncycle01 = 0;
 int Ncycle12 = 0;
 int Ncycle23 = 0;
 int Ncycle34 = 0;
 unsigned int d1=1,d2=2,d3=3,d4=4;
  
 TCNT1 = 0;  
  while (ACSR &(1<<ACO)){
    if (TIFR1&(1<<TOV1)) {
      Nsignal--;
      TIFR1 = _BV(TOV1);
      if (Nsignal <= 0) {break;}
    }
  }
  while ((ACSR &(1<<ACO))==0){
    if (TIFR1&(1<<TOV1)) {
      Nsignal--;
      TIFR1 = _BV(TOV1);
      if (Nsignal <= 0) {break;}
    }
  }
  if (Nsignal <= 0) {break;}

  TCNT1 = 0;
  while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle01++;
      TIFR1 = _BV(TOV1);
      if (Ncycle01 >= 2) {break;}
    }
  }
  d1 = ICR1;  
  while ((ACSR &(1<<ACO))==0){
      if (TIFR1&(1<<TOV1)) {
      Ncycle12++;
      TIFR1 = _BV(TOV1);
      if (Ncycle12 >= 3) {break;}      
    }
  } 
  while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle12++;
      TIFR1 = _BV(TOV1);
      if (Ncycle12 >= 6) {break;}
    }
  }
  d2 = ICR1;
  while ((ACSR &(1<<ACO))==0){
      if (TIFR1&(1<<TOV1)) {
      Ncycle23++;
      TIFR1 = _BV(TOV1);
      if (Ncycle23 >= 3) {break;}
    }
  } 
  while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle23++;
      TIFR1 = _BV(TOV1);
      if (Ncycle23 >= 6) {break;}
      }
  } 
  d3 = ICR1;
  while ((ACSR &(1<<ACO))==0){
      if (TIFR1&(1<<TOV1)) {
      Ncycle34++;
      TIFR1 = _BV(TOV1);
      if (Ncycle34 >= 3) {break;}
    }
  } 
  while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle34++;
      TIFR1 = _BV(TOV1);
      if (Ncycle34 >= 6) {break;}
      }
  } 
  d4 = ICR1;
 
  unsigned long codefreq1 = 1600000000/(65536*Ncycle12+d2-d1);
  unsigned long codefreq2 = 1600000000/(65536*Ncycle23+d3-d2);
  unsigned long codefreq3 = 1600000000/(65536*Ncycle34+d4-d3);
  unsigned long codefreq = (codefreq1 + codefreq2 + codefreq3)/3;
  if (d3==d4) codefreq = 5000;
     
  if ((codefreq < 310000) and  (codefreq >= 10000)) {
      if (FSKtx == 0 & freqcheck(freq)==0){
          digitalWrite(13,1);
          digitalWrite(11,0);  //RX switch off
          digitalWrite(12,0);
          delay(10);
          si5351.output_enable(SI5351_CLK1, 0);   //RX off
          si5351.output_enable(SI5351_CLK0, 1);   //TX on
          lcd.setCursor(0,7);
          lcd.print("T");
      }
      si5351.set_freq((freq*100ULL + codefreq), SI5351_CLK0);  
      FSKtx = 1;
    }
  else{
    FSK--;
  }
 }

  si5351.output_enable(SI5351_CLK0, 0);   //TX off
  lcd.setCursor(0,7);
  lcd.print("R");
  if(Serial.available() > 0) cat();
  si5351.set_freq(freq*100ULL, SI5351_CLK1);
  delay(10);
  digitalWrite(13,0);  
  digitalWrite(11,1);  //RX switch on
  digitalWrite(12,1);
  si5351.output_enable(SI5351_CLK1, 1);   //RX on
  FSKtx = 0;
}

//remote contol (simulating TS-2000)
//"ft8qrp_cat11.ico" from https://www.elektronik-labor.de/HF/FT8QRP.html
//slightly modified
void cat(void)
{     
String received;
String receivedPart1;
String receivedPart2;    
String command;
String command2;  
String parameter;
String parameter2; 
String sent;
String sent2;
    received = Serial.readString();  
    received.toUpperCase();  
    received.replace("\n","");  

           String data = "";
           int bufferIndex = 0;

          for (int i = 0; i < received.length(); ++i)
               {
               char c = received[i];
    
                 if (c != ';')
                    {
                    data += c;
                    }
                 else
                    {
                        if (bufferIndex == 0)
                          {  
                              data += '\0';
                              receivedPart1 = data;
                              bufferIndex++;
                              data = "";
                          }
                         else
                          {  
                              data += '\0';
                              receivedPart2 = data;
                              bufferIndex++;
                              data = "";
                          }
                    }

               }
    
    command = receivedPart1.substring(0,2);
    command2 = receivedPart2.substring(0,2);    
    parameter = receivedPart1.substring(2,receivedPart1.length());
    parameter2 = receivedPart2.substring(2,receivedPart2.length());

    sent = String("?;"); //added
    sent2 = String("?;"); //added

    if (command == "FA")  
    {  
        
          if (parameter != "")  
              {  
              freq = parameter.toInt();
              freqchangeflag=1;  //added
              //VfoRx = VfoTx;   
              }
          
          sent = "FA" // Return 11 digit frequency in Hz.  
          + String("00000000000").substring(0,11-(String(freq).length()))   
          + String(freq) + ";";     
    }

    else if (command == "PS")  
        {  
        sent = "PS1;";
        }

    else if (command == "TX")  
        {   
        sent = "TX0;";
        //VfoTx = freq;
        //digitalWrite(13,1);
        TxStatus = 1;
       
        }

    else if (command == "RX")  
        {  
        sent = "RX0;";
        //VfoRx = freq;
        //digitalWrite(13,0);
        TxStatus = 0;
        
        }

    else  if (command == "ID")  
        {  
        sent = "ID019;";
        }

    else if (command == "AI")  
        {
        sent = "AI0;"; 
        }

    else if (command == "IF")  
        {
          if (TxStatus == 1)
            {  
            sent = "IF" // Return 11 digit frequency in Hz.  
            + String("00000000000").substring(0,11-(String(freq).length()))   
            //+ String(freq) + String("     ") + "+" + "0000" + "0" + "0" + "0" + "00" + "0" + String(mode) + "0" + "0" + "0" + "0" + "00" + String(" ") + ";"; 
            + String(freq) + "00000" + "+" + "0000" + "0" + "0" + "0" + "00" + "1" + String(mode) + "0" + "0" + "0" + "0" + "000" + ";"; 
            } 
             else
            {  
            sent = "IF" // Return 11 digit frequency in Hz.  
            + String("00000000000").substring(0,11-(String(freq).length()))   
            //+ String(freq) + String("     ") + "+" + "0000" + "0" + "0" + "0" + "00" + "0" + String(mode) + "0" + "0" + "0" + "0" + "00" + String(" ") + ";"; 
            + String(freq) + "00000" + "+" + "0000" + "0" + "0" + "0" + "00" + "0" + String(mode) + "0" + "0" + "0" + "0" + "000" + ";"; 
            } 
       }
  
    else if (command == "MD")  
      {  
      sent = "MD2;";
      }

//------------------------------------------------------------------------------      

    if (command2 == "ID")  
        {  
        sent2 = "ID019;";
        }
            
    if (bufferIndex == 2)
        {
        Serial.print(sent2);
        }
        
    else
    {
        Serial.print(sent);
    }  

   if ((command == "RX") or (command = "TX")) delay(50);

    sent = String("");
    sent2 = String("");  
}


//LCD frequency display (8x2)
void lcd_disp(long int frequency, int cursolposition)
{
  lcd.setCursor(1,0);
  lcd.print("        ");
  String freqString =  String(frequency, DEC);
  lcd.setCursor(1,8-freqString.length());
  lcd.print(freqString);
  lcd.setCursor(1,cursolposition-1);
}


//prevention of out-of-band transmission (in JA)
long int freq_in_band(long int frequency, long int freqset)
{
  long int freq_in_band;
  if (freqset < 135700) {
    freq_in_band=135700;
    return freq_in_band;
  }
  else if (freqset > 135800 && freqset < 472000) {
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

//checking for the prevention of out-of-band transmission (in JA)
int freqcheck(long int frequency)  // retern 1=out-of-band, 0=in-band
{
  if (frequency < 135700) {
    return 1;
  }
  else if (frequency > 135800 && frequency < 472000) {
    return 1;
  }
  else if (frequency > 479000 && frequency < 1800000) {
    return 1;
  }
  else if (frequency > 1875000 && frequency < 1907500) {
    return 1;
  }
  else if (frequency > 1912500 && frequency < 3500000) {
    return 1;
  }
  else if (frequency > 3580000 && frequency < 3662000) {
    return 1;
  }
  else if (frequency > 3687000 && frequency < 3716000) {
    return 1;
  }
  else if (frequency > 3770000 && frequency < 3791000) {
    return 1;
  }
  else if (frequency > 3805000 && frequency < 7000000) {
    return 1;
  }
  else if (frequency > 7200000 && frequency < 10100000) {
    return 1;
  }
  else if (frequency > 10150000 && frequency < 14000000) {
    return 1;
  }
  else if (frequency > 14350000 && frequency < 18068000) {
    return 1;
  }
  else if (frequency > 18168000 && frequency < 21000000) {
    return 1;
  }
  else if (frequency > 21450000 && frequency < 24890000) {
    return 1;
  }
  else if (frequency > 24990000 && frequency < 28000000) {
    return 1;
  }
  else if (frequency > 29700000 && frequency < 50000000) {
    return 1;
  }
  else if (frequency > 54000000) {
    return 1;
  }
  else return 0;
}
