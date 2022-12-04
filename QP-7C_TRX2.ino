// ==============Display&RotaryEncorder=============
#define IhaveOLED&RotaryEncorder
// =================================================
// ==================CatControll====================
#define IuseCatControll
// =================================================
// =====MakerNanoBuzzer for CW tone (D8 pin)========
//#define IuseMakerNanoBuzzer
// =================================================
// =Decentralize Beats to white-like noise(CW mode)=
//#define IuseBeatDecentralization
// =================================================

//#define FREQ_CW   3560000 // in Hz
#define FREQ_CW   7003000 // in Hz
//#define FREQ_CW   7030000 // in Hz
//#define FREQ_CW   10106000 // in Hz
//#define FREQ_CW   14060000 // in Hz
//#define FREQ_CW   18096000 // in Hz
//#define FREQ_CW   21060000 // in Hz
//#define FREQ_CW   24906000 // in Hz
//#define FREQ_CW   28060000 // in Hz
//#define FREQ_CW   50096000 // in Hz

#define CW_TONE 600 // +(CW), -(CWR)

//#define FREQ_DIGI 3531000 //FT8 (JA local) in Hz
//#define FREQ_DIGI 3573000 //FT8 (DX) in Hz
#define FREQ_DIGI 7041000 //FT8 (JA local) in Hz
//#define FREQ_DIGI 7074000 //FT8 (DX) in Hz
//#define FREQ_DIGI 10136000 //FT8 in Hz
//#define FREQ_DIGI 14074000 //FT8 in Hz
//#define FREQ_DIGI 18100000 //FT8 in Hz
//#define FREQ_DIGI 21074000 //FT8 in Hz
//#define FREQ_DIGI 24915000 //FT8 in Hz
//#define FREQ_DIGI 28074000 //FT8 in Hz
//#define FREQ_DIGI 50313000 //FT8 in Hz

#define BFO_LSB 453500 // in Hz: Calibrate for your Ceramic Filter.
#define BFO_USB 446400 // in Hz: Calibrate for your Ceramic Filter.
//#define BFO_LSB 451040 // in Hz: Calibrate for your CERATA Filter.
//#define BFO_USB 449840 // in Hz: Calibrate for your CERATA Filter.
//#define BFO_LSB 450000 // in Hz
//#define BFO_USB 450000 // in Hz

#define Si5351_CAL -22000 // Calibrate for your Si5351 module.

#include <si5351.h>
#include <Wire.h>

#ifdef IhaveOLED&RotaryEncorder
  //OLED
  #include "SSD1306Ascii.h"
  #include "SSD1306AsciiAvrI2c.h"
  #define I2C_ADDRESS 0x3c
  SSD1306AsciiAvrI2c oled;
  //Rotary encoder
  #include <Rotary.h>
  Rotary r = Rotary(10, 9);
#endif

Si5351 si5351;

int local = 1 ; // 1: reciving freq = local osc. freq + IF , -1: reciving freq = local osc.freq - IF 
long int freq=FREQ_DIGI;   //initial frequency (Hz)
long int freqstep = 1000;  //initial frequency step (Hz)
long int bfofreq=BFO_USB;  //initial BFO frequency step (Hz)
int cursol = 5;            // =8-log(freqstep)
int mode; //0=CW mode (single key), 1=CW mode (puddle), 2=Digital mode
int cw_tone;
int cw_rate= 20; //WPM
int TxStatus = 0; //0=RX, 1=TX
int key1;
int key2;
int oled_changed; //1 = oled is changed (for s meter display)

void setup(void)
{
  Serial.begin(115200); 
  Serial.setTimeout(4);
  //si5351 initialization-----  
  int32_t cal_factor = Si5351_CAL;
  //int32_t cal_factor = 0;
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0); 
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(freq*100ULL, SI5351_CLK0);  //for TX
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.set_freq((freq-bfofreq*local)*100ULL, SI5351_CLK1);  //for RX Mix
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);
  si5351.output_enable(SI5351_CLK2, 0);
  si5351.set_freq((bfofreq-cw_tone*local)*100ULL, SI5351_CLK2);  //for RX BFO
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
  si5351.output_enable(SI5351_CLK1, 0);
#ifdef IhaveOLED&RotaryEncorder
  //OLED initialization----- 
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  //Rotary encoder initialization-----
  r.begin();
#endif   

  //Digital pins ----- 
  pinMode(2, INPUT_PULLUP); //key1 (dash-Key or single Key)
  pinMode(4, INPUT); //Paddle mode →　OUTPUT & High
  pinMode(5, INPUT_PULLUP); //key2 (dot-Key)
  pinMode(6, INPUT); //PD7 = AN1 = HiZ, PD6 = AN0 = 0
  pinMode(7, INPUT); //PD7 = AN1 = HiZ, PD6 = AN0 = 0
  //pinMode(9, INPUT_PULLUP); //Rotary encoder
  //pinMode(10, INPUT_PULLUP); //Rotary encoder
  pinMode(11, INPUT_PULLUP); //SW (freq. step down)
  pinMode(12, OUTPUT); //RX →　High, TX →　Low (for RX switch)
  pinMode(13, OUTPUT); //TX →　High, RX →　Low (for Driver switch)


  //Mode check at power-up-----  
  delay(100);
  if (digitalRead(11)==0){
      mode = 2; //Digital(FSK) mode
      digital_prep();
  }
  else {
    key1=digitalRead(2);
    key2=digitalRead(5)*2; 

    switch (key1+key2){
    case 1:
      mode = 0; //Single key mode (CW)
      cw_prep();
      break;
    case 2:
      mode = 0; //Single key mode (CW)
      cw_prep();
      break;
    case 3:
      mode = 1; //Paddle mode (CW)
      cw_prep();
      pinMode(4, OUTPUT);
      digitalWrite(4,1); //Pull up 5 pin (dash pin) with 1 kOhm resistor
      break;
    default:
      mode = 2; //Digital(FSK) mode
      cw_tone=0;
    }
  }
  
  #ifdef IhaveOLED&RotaryEncorder
  oled_disp();
  #endif   
 
  si5351_freqset();
  si5351.output_enable(SI5351_CLK0, 0);   //TX osc. off
  si5351.output_enable(SI5351_CLK1, 1);   //RX Mix osc. on
  si5351.output_enable(SI5351_CLK2, 1);   //RX BFO osc. on
  digitalWrite(12,1);  //RX on
  digitalWrite(13,0);  //TX off
}

void cw_prep(void){
  freq=FREQ_CW;
  if (freq < 10000000) {
    cw_tone = -CW_TONE;
    if (local = 1) bfofreq = BFO_LSB;
    else bfofreq = BFO_USB;
  }
  else {
    cw_tone = CW_TONE;
    if (local = 1) bfofreq = BFO_USB;
    else bfofreq = BFO_LSB;
  }
}

void digital_prep(void){
  freq=FREQ_DIGI;        
  cw_tone=0;
  if (local = 1) bfofreq = BFO_USB;
  else bfofreq = BFO_LSB;
  //Timer setting for audio frequency analysis----- 
  TCCR1A = 0x00;
  TCCR1B = 0x01; // Timer1 Timer 16 MHz
  TCCR1B = 0x81; // Timer1 Input Capture Noise Canceler
  ACSR |= (1<<ACIC);  // Analog Comparator Capture Input
}

void loop(void)
{
  if (mode == 0) cw0();
  else if (mode == 1) cw1();
  else digital();
}

void cw0(void) //Single key mode (CW)
{
  // Keying
  if (digitalRead(2)==LOW) {
    cw_tx();
  }
  else {
    cw_rx();
    //s=analogRead(A0)/3.2;
    //oled_disp_s(s);
  }
  // change the frequency by rotary encoder
  #ifdef IhaveOLED&RotaryEncorder
    cwfreqchange();
  #endif  
  // change the frequency by remote control 
  #ifdef IuseCatControll
    if(Serial.available() > 0) cat();
  #endif
  #ifdef IuseBeatDecentralization
    delayMicroseconds(random(100)); //Apply a random delay of 0-100uS to decentralize digitalRead() beats to white-like noise.
  #endif
}

void cw1(void) //Paddle mode (CW)
{
  int cw_length = int(1200/cw_rate); //ms = 1200/cw_rate
  key1=digitalRead(2);
  key2=digitalRead(5);
  if (key2==0){
    cw_tx();
    key1=digitalRead(2);
    for (int i = 0; i < 15; i++) {
    delay(cw_length/5);
    key1=key1 + digitalRead(2);
    }
    cw_rx();
    delay(cw_length);
    if (key1 <= 15){
      cw_tx();
      delay(cw_length);
      cw_rx();
      delay(cw_length);
    }
  }
  else if  (key1==0){
    cw_tx();
    delay(cw_length);
    cw_rx();
    delay(cw_length);
  }  
  else {   
    // change the frequency by rotary encoder
    #ifdef IhaveOLED&RotaryEncorder    
      cwfreqchange();
    #endif  
    // change the frequency by remote control 
    #ifdef IuseCatControll
      if(Serial.available() > 0) cat();
    #endif  
  }
  #ifdef IuseBeatDecentralization
    delayMicroseconds(random(100)); //Apply a random delay of 0-100uS to decentralize digitalRead() beats to white-like noise.
  #endif
}

void cw_tx(void){
  if (TxStatus==0 && freqcheck(freq)==0) {
    //si5351.output_enable(SI5351_CLK1, 0);   //RX Mix osc. off
    //si5351.output_enable(SI5351_CLK2, 0);   //RX BFO osc. off
    si5351.output_enable(SI5351_CLK0, 1);   //TX osc. on
    digitalWrite(12,0);   //RX off
    digitalWrite(13,1);   //TX on
    #ifdef IuseMakerNanoBuzzer
      tone(8, cw_tone);   //Maker Nano buzzer on
    #endif
    TxStatus=1;
  }  
}

void cw_rx(void){
  if (TxStatus==1) {
    si5351.output_enable(SI5351_CLK0, 0);   //TX osc. off
    si5351.output_enable(SI5351_CLK1, 1);   //RX Mix osc. on
    si5351.output_enable(SI5351_CLK2, 1);   //RX BFO osc. on
    digitalWrite(13,0);   //TX off
    digitalWrite(12,1);   //RX on
    #ifdef IuseMakerNanoBuzzer
      noTone(8);   //Maker Nano buzzer off
    #endif
    TxStatus=0;
  }
}

void digital(void)
{
  //https://www.elektronik-labor.de/HF/FT8QRP.html
  //partly modified
  //(Using 3 cycles for timer sampling to improve the precision of frequency measurements)
  //(Against overflow in low frequency measurements)
  // change the frequency by rotary encoder
  #ifdef IhaveOLED&RotaryEncorder
    digitalfreqchange();
  #endif  
  // change the frequency by remote control 
  #ifdef IuseCatControll
    if(Serial.available() > 0) cat();
  #endif 
 
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
      if (FSKtx == 0 && freqcheck(freq)==0){
      
        digitalWrite(12,0);   //RX off
        digitalWrite(13,1);   //TX on
        delay(10);
        si5351.output_enable(SI5351_CLK1, 0);   //RX Mix osc. off
        si5351.output_enable(SI5351_CLK2, 0);   //RX BFO osc. off
        si5351.output_enable(SI5351_CLK0, 1);   //TX osc. on
      }
      si5351.set_freq((freq*100ULL + codefreq), SI5351_CLK0);  
      FSKtx = 1;
    }
    else{
      FSK--;
    }
  }
  
  if (FSKtx == 1) {  
    si5351.output_enable(SI5351_CLK0, 0);   //TX osc. off
    delay(10);
    digitalWrite(13,0);  //TX off
    digitalWrite(12,1);  //RX on
    si5351.output_enable(SI5351_CLK1, 1);   //RX Mix osc. on
    si5351.output_enable(SI5351_CLK2, 1);   //RX BFO osc. on
    FSKtx = 0;
  }

  // change the frequency by rotary encoder
  #ifdef IhaveOLED&RotaryEncorder
    digitalfreqchange();
  #endif  
  // change the frequency by remote control 
  #ifdef IuseCatControll
    if(Serial.available() > 0) cat();
  #endif 
}


#ifdef IhaveOLED&RotaryEncorder
//frequency change by rotary encoder
void cwfreqchange(void)
{
  long int freqset;
  int r_result;
  //int temp=1;

  // change the frequency step by SW
  if (digitalRead(11) == LOW){
  //if (temp == 0){
    delay(200);
    if (digitalRead(11) == LOW && mode==1){
      oled_disp_cw_rate();    
      while (digitalRead(11)==LOW){  
        r_result=r.process();
        if (r_result==DIR_CW){
          cw_rate++;
          oled_disp_cw_rate();    
        }
        else if (r_result==DIR_CCW){
          cw_rate--;
          oled_disp_cw_rate();    
        }
      }
    }
    else{
      cursol++;
      freqstep = freqstep / 10;
      if (cursol>7) {
        cursol=2;
        freqstep = 1000000;
      }
    }
    oled_disp();
  } 
  // read rotary encoder state
  r_result=r.process();
  // increase the frequency by rotary encoder
  if (r_result==DIR_CW){
    freqset = freq + freqstep;
    if (freqset <= 54000000) freq = freqset;
    if (freq < 10000000) {
      cw_tone = -CW_TONE;
      if (local = 1) bfofreq = BFO_LSB;
      else bfofreq = BFO_USB;
    }
    else {
      cw_tone = CW_TONE;
      if (local = 1) bfofreq = BFO_USB;
      else bfofreq = BFO_LSB;
    }
    si5351_freqset();
    oled_disp();
  }
  // decrease the frequency by by rotary encoder
  else if (r_result==DIR_CCW){
    freqset = freq - freqstep;
    if (freqset >= 1000000) freq = freqset;
    if (freq < 10000000) {
      cw_tone = -CW_TONE;
      if (local = 1) bfofreq = BFO_LSB;
      else bfofreq = BFO_USB;    
    }
    else {
      cw_tone = CW_TONE;
      if (local = 1) bfofreq = BFO_USB;
      else bfofreq = BFO_LSB;
    }
    si5351_freqset();
    oled_disp();
  }
}

void digitalfreqchange(void)
{
  long int freqset;
  int r_result;
  // change the frequency step by SW
  if (digitalRead(11)==LOW){
    delay(200);
    if (digitalRead(11)==LOW){
      while (digitalRead(11)==LOW){      
        r_result=r.process();
        if (r_result==DIR_CW){
          freqset = freq + freqstep;
          if (freqset <= 54000000) freq = freqset;
          si5351_freqset();
          oled_disp();
        }
        else if (r_result==DIR_CCW){
          freqset = freq - freqstep;
          if (freqset >= 1000000) freq = freqset;
          si5351_freqset();
          oled_disp();
        }
      }
    }
    else {
      cursol++;
      freqstep = freqstep / 10;
      if (cursol>5) {
        cursol=2;
        freqstep = 1000000;
      }
      oled_disp();
    }
  }
}

void si5351_freqset(void){
  si5351.set_freq(freq*100ULL, SI5351_CLK0); //TX
  si5351.set_freq((freq-bfofreq*local)*100ULL, SI5351_CLK1); //RX Mix  
  si5351.set_freq((bfofreq-cw_tone*local)*100ULL, SI5351_CLK2);   //RX BFO freequency
}

//OLED frequency display (128x64)
void oled_disp() {
  String freqString =  String(freq, DEC);
  String modeString;
  int cursolposition;
  switch(mode){
    case 0:
      modeString = "CW(single)";
      break;
    case 1:
      modeString = "CW(paddle)";
      break;
    case 2:
      modeString = "  FSK mode";
      break;
    default:
      modeString = "";
  }  
  switch(freqString.length()){
    case 8:
      freqString = freqString.substring(0,2)+","+freqString.substring(2,5)+","+freqString.substring(5,8);
      break;
    case 7:
      freqString = " "+freqString.substring(0,1)+","+freqString.substring(1,4)+","+freqString.substring(4,7);
      break;
    case 6:
      freqString = "   "+freqString.substring(0,3)+","+freqString.substring(3,6);
      break;
    default:
      freqString = freqString;
  }
   
  //cursolposition = cursol-1;
  cursolposition = cursol;
  if(cursol>2) cursolposition = cursolposition +1 ;
  if(cursol>5) cursolposition = cursolposition +1 ;

  oled.clear();
  oled.setFont(X11fixed7x14);
  oled.setCursor(0, 0);
  oled.println("QP-7C");
  oled.setCursor(45, 1);
  oled.println(modeString);
  oled.setFont(System5x7);  
  oled.setCursor(0, 3);
  oled.set2X();
  oled.println(freqString);
  oled.setCursor(0, 5);
  for (int ii = 0; ii < cursolposition-1; ii +=1) {
    oled.print(" ");
  }
  oled.println("=");
  oled.set1X();
  oled.setCursor(107, 6);
  oled.setFont(X11fixed7x14);
  oled.println("Hz");
  oled_changed=1;
}

//OLED cw_rate display (128x64)
void oled_disp_cw_rate()
{
  oled.clear();
  oled.setFont(X11fixed7x14);
  oled.setCursor(0, 0);
  oled.println("QP-7C");
  oled.setCursor(45, 1);
  oled.println("CW speed");
  oled.setFont(System5x7);  
  oled.setCursor(30, 4);
  oled.set2X();
  oled.print(cw_rate);
  oled.println(" WPM");
  oled.set1X();
}

#endif 

#ifdef IuseCatControll
//remote contol (simulating TS-2000)
//"ft8qrp_cat11.ico" from https://www.elektronik-labor.de/HF/FT8QRP.html
//slightly modified
void cat(void) {     
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

  if (command == "FA") {          
    if (parameter != "")  
    {  
      // freq = parameter.toInt();
      // added from here             
      long int freqset = parameter.toInt();
      if (freqset >= 1000000 && freqset <= 54000000) freq = freqset;
      if (mode == 2) cw_tone = 0;
      else if (freq < 10000000) { 
        cw_tone = -CW_TONE;
      if (local = 1) bfofreq = BFO_LSB;
      else bfofreq = BFO_USB;    
      }
      else {
        cw_tone = CW_TONE;
        if (local = 1) bfofreq = BFO_USB;
        else bfofreq = BFO_LSB;    
      }
      si5351_freqset();
      #ifdef IhaveOLED&RotaryEncorder
      oled_disp();
      #endif
      // to here            
      //VfoRx = VfoTx;   
    }          
    sent = "FA" // Return 11 digit frequency in Hz.  
    + String("00000000000").substring(0,11-(String(freq).length()))   
    + String(freq) + ";";     
  }
  else  if (command == "ID")  {  
    sent = "ID019;";
  }
  else  {
    sent = String("?;"); //added
  }
//------------------------------------------------------------------------------ 
  if (command2 == "ID")   {  
    sent2 = "ID019;";
  }
  else  {
    sent2 = String("?;"); //added
  }               
  
  if (bufferIndex == 2)  {
    Serial.print(sent2);
  }        
  else  {
    Serial.print(sent);
  }  
}
#endif

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