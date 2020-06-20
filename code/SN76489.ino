#include "mario_theme.h"
#include "mario_gameover.h"
#include "mario_underworld.h"
#include "mario_starman.h"
#include "mario_death.h"

#define CLOCK_PIN 11 // D11 - OCR2A
#define CHIP_EN 8 // D8
#define CLOCK_FREQ_MHZ 2000000L // 2 MHz
#define READY_PIN 2 // D2 
#define WRITE_EN 3 // D3

#define DATA0 14 //A0
#define DATA1 15 //A1
#define DATA2 16 //A2
#define DATA3 17 //A3
#define DATA4 4 // D4
#define DATA5 5 // D5
#define DATA6 6 // D6
#define DATA7 7 // D7

// Playback global used to specific the song playing:
struct MIDIsong *current_song=0; // pointer to MIDIsong structure
uint16_t current_num = 0;
long last_millis = 0;
long channel_start[3] = {0,0,0}; // absolute time in milliseconds
uint16_t channel_duration[3] = {0,0,0};
long last_beat=0;
uint8_t beating=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(READY_PIN, INPUT_PULLUP);
  
  digitalWrite(WRITE_EN, HIGH);
  digitalWrite(CHIP_EN, HIGH);
  
  pinMode(WRITE_EN, OUTPUT);
  pinMode(CHIP_EN, OUTPUT);
  
  // Data output pins:
  pinMode(DATA0, OUTPUT); // LSB
  pinMode(DATA1, OUTPUT); 
  pinMode(DATA2, OUTPUT); 
  pinMode(DATA3, OUTPUT); 
  pinMode(DATA4, OUTPUT); 
  pinMode(DATA5, OUTPUT); 
  pinMode(DATA6, OUTPUT); 
  pinMode(DATA7, OUTPUT); // MSB
 
  Serial.print("Setting clock output:");
  Serial.println(CLOCK_FREQ_MHZ);

   // Set up the high frequency clock output
  
    pinMode(CLOCK_PIN, OUTPUT);
    
    // Use CTC mode:
    // freq = F_CPU / 2*N*(OCR0A+1)
    // From the datasheet this appears to be classified as a "non-PWM mode"
    //
    // There is no error checking in the below. A maximum requested clock would have OCR0A = 255
    // meaning an output frequency of F_CPU/(2*256), i.e. 31.25 kHz @ 16 MHz F_CPU
    // Frequencies requested that are slower will overflow OCR0A and cause unpredictable results!

    // Also 2*CLOCK_FREQ_MHZ MUST BE DIVISIBLE INTO F_CPU, i.e. if F_CPU = 16 MHz:
    // OCR0A = 0, CLOCK_FREQ_MHz = 8 MHz
    // OCR0A = 1, CLOCK_FREQ_MHZ = 4 MHz
    // OCR0A = 2, CLOCK_FREQ_MHZ = 2.66 MHz
    // OCR0A = 3, CLOCK_FREQ_MHZ = 2 MHz
    // ...
    // OCR0A = 7, CLOCK_FREQ_MHZ = 1 MHz
    // etc

    // From the datasheet: "The waveform generated will have a maximum frequency of fOC0 = fclk_I/O/2 
    // when OCR0A is set to zero (0x00).
    
    // CS00 set = ClkI/O / 1 i.e. no prescaling on system clock
    // WGM01 set = Mode 2 = CTC. TOP = OCRA, Update OCRx at "Intermediate"
    // COM0A0 = OC0A output is toggled in CTC mode ("non-PWM Mode")
    // COM0B0 = OC0B output is toggled in CTC mode

    /*
    TCCR0A = _BV(COM0B0) | _BV(WGM01);
    TCCR0B = _BV(CS00);

    OCR0A = (F_CPU/(2*CLOCK_FREQ_MHZ))-1; // OCRA defined in CTC mode as the top
    OCR0B = 0;
  Serial.print(", OCR0A=");
    Serial.print(OCR0A);*/

    // Use Timer2 to clock the SN76489 chip, leaving millis on Timer0 free for use :)
    TCCR2A = _BV(COM2A0) | _BV(WGM21);
    TCCR2B = _BV(CS20);

    OCR2A = (F_CPU/(2*CLOCK_FREQ_MHZ))-1; // OCRA defined in CTC mode as the top
    OCR2B = 0;
    
    Serial.print("OCR2A="); Serial.println(OCR2A);
}

void StartSong(struct MIDIsong *song)
{
  uint16_t resolution, milli_pqn, num_notes;
  
  current_song = song;
  resolution = current_song->resolution; 
  milli_pqn = current_song->milli_pqn;
  num_notes = current_song->num_notes;
  
  current_song->curr_note = song->first_note;
  current_num = 0;
  last_millis = millis();
  last_beat = last_millis;
  beating = 0;
}

uint8_t PlayMusic(void)
{
  struct MIDInote *note; // local pointer makes code more readable
  
  if (current_song==0) {Serial.println("Error! No song playing!"); return 1;}
  // Get parameters for current note:
  note = current_song->curr_note;

  uint8_t qrt;
  uint16_t rel_millis;

  // Check if notes in the channels have expired
  for (uint8_t c=0;c<3;c++)
  {
    if (channel_duration[c]>0) // Then means the channel is active
    if ((millis()-channel_start[c])>=channel_duration[c]) // Note has expired on this channel
    {
    // Switch off channel c (attentuation = 0x0f)
    SendByte_SN76490((1<<7) | (c<<5) | (1<<4) | (0b1111));
    //Serial.print("Channel OFF:"); Serial.print(c);Serial.print(:");
    //Serial.print(millis()); Serial.print(":"); Serial.print(channel_start[c]); Serial.print(":"); Serial.print(channel_duration[c]);
    channel_duration[c] = 0; // means channel is free

    //Serial.print("Note OFF on channel:"); Serial.println(c);
    }
  }

  if (current_num>=current_song->num_notes) // reached the end of the song
  {
       if (channel_duration[0]+channel_duration[1]+channel_duration[2]==0) // no channels still playing final notes
       {
        current_song = 0; // Nullify the pointer
        SendByte_SN76490(0b11111111); // Stop the noise channel
        return 1;  // Means the song has stopped (or there was an error)
       }
       return 0;
  }

  if (millis()-last_beat>=2*current_song->milli_pqn)
  {
    // Start beat
    SendByte_SN76490(0b11110000); // Noise channel on
    SendByte_SN76490(0b11100100); // Noise channel configure NF0 = NF1 = 0 = N/512 shift rate, FB = 1 = White noise
    
    //Serial.println("*");
    
    last_beat=millis();
    beating = 1;
  }
  
  if (beating) // stop drum beat
  {
    if (millis()-last_beat>=20) // Beat length = 50 ms
    {
      SendByte_SN76490(0b11111111); // Noise channel off
      beating = 0;
    }
  }
  
  // Is it time to play the pending note? 
  qrt = pgm_read_byte(&note->quant_reltick);
  rel_millis=((uint32_t)qrt*32*current_song->milli_pqn/current_song->resolution);
  
  if ((millis()-last_millis)>=(rel_millis))  // Time has elapsed, so play note on a free channel 
  {
    // Find channel:
    uint8_t select_channel=0;
    for (uint8_t c=0;c<3;c++)
    {
      if (channel_duration[c]==0) {select_channel=c+1; break;}
    }
    
    if (select_channel>0)
    {
      select_channel--;
      //Serial.print("Note ON on channel:"); Serial.print(select_channel);Serial.print(":");
      uint8_t d; 
      uint16_t duration_millis;
      
      d = pgm_read_byte(&note->duration);  
      duration_millis = ((uint16_t)current_song->milli_pqn * 4)/(uint16_t)d;
      duration_millis *= 0.95f; 
      //Serial.print(":"); Serial.print(d); Serial.print(":");Serial.print(duration_millis);Serial.print("#");
      PlayNote(note, select_channel);
     
      last_millis = millis();
      channel_start[select_channel] = last_millis; // Save start time in millis
      channel_duration[select_channel] = duration_millis;
      
      current_song->curr_note++; // Advance to next note
      Serial.println(current_num);
      current_num++;
    }
    else
    {
      Serial.println("No free channels... waiting...");
    }       
  } // time to play a new note.
  return 0; // 0 means we're still playing the song
}

void SendByte_SN76490(uint8_t data)
{

  digitalWrite(CHIP_EN, LOW); // Enable CHIP (active low)
  
  while (digitalRead(READY_PIN)){};  // Wait until READY is LOW 
  
  // Put data on data bus. 
  digitalWrite(DATA7, data & (1<<0) ? HIGH:LOW);
  digitalWrite(DATA6, data & (1<<1) ? HIGH:LOW);
  digitalWrite(DATA5, data & (1<<2) ? HIGH:LOW);
  digitalWrite(DATA4, data & (1<<3) ? HIGH:LOW);
  digitalWrite(DATA3, data & (1<<4) ? HIGH:LOW);
  digitalWrite(DATA2, data & (1<<5) ? HIGH:LOW);
  digitalWrite(DATA1, data & (1<<6) ? HIGH:LOW);
  digitalWrite(DATA0, data & (1<<7) ? HIGH:LOW);
  
  // Pulse WRITE_EN
  digitalWrite(WRITE_EN,LOW);
  delayMicroseconds(100);
  digitalWrite(WRITE_EN,HIGH);
 
  // Wait until READY is HIGH 
  digitalWrite(CHIP_EN,HIGH);
  while(!digitalRead(READY_PIN)){};// Make sure READY is HIGH
}

void PlayNote(struct MIDInote *note, uint8_t channel)
{
    uint8_t p,v;
    
    float freq;
    uint16_t n10bit;
    uint8_t scaled_vel,atten; // Take volume and convert to attenuation (0-15)
    
    p = pgm_read_byte(&note->pitch);       
    v = pgm_read_byte(&note->velocity);

    // pitch is midi note 0-127
    // velocity is volume essentially 0-127
      
    freq = (p-69)/12.0f;
    freq = pow(2,freq)*440.0f;
    // Now we have the frequency, calculate the 10-bit number to send to the SN76489 
    n10bit = int(round(CLOCK_FREQ_MHZ/(freq*32.0)));

    if (n10bit>1023) 
    {
      Serial.println("Error! Pitch cannot be represented in 10-bits with this CLOCK_FREQ_MHZ!");
      //while(1);
    }
    // The attenuation bit pattern is somewhat the inverse of the volume
    // the more bits, the greater the attenuation (the lower the volume)

   // equivalent to:
   // A3 A2 A1 A0
   // 0  0  0  0   0 = 0 db = Full volume i.e. MIDI velocity = 127
   // 1  0  0  0   8 = -2 db 
   // 0  1  0  0   4 = -4 db
   // 0  0  1  0   2 = -8 db
   // 0  0  0  1   1 = -16 db 
   // 1  1  1  1   15 = CHANNEL OFF - i.e. MIDI velocity = 0

  v = v&127; // make sure volume is within expected range
  atten = (127-v); // Atten = 0 for 127 volume, and 127 for 0 volume
  atten >>= 3;

  atten = 0; // Max volume!
  
  // We've now calculated the 10-bits to describe the frequency, and the attenuation bits
  // and we were told the channel, this is enough to send off a byte for writing!
  uint8_t byte_one=0, byte_two=0, byte_three=0;

  byte_one= (1<<7) | (channel<<5) | (n10bit & 0b1111);
  byte_two = (n10bit>>4) & 0b00111111;
  byte_three = (1<<7) | (channel<<5) | (1<<4) | (atten & 0b1111);

  //Serial.print(channel); Serial.print(","); Serial.print(n10bit); Serial.print(","); Serial.print(atten); Serial.print(",");
  SendByte_SN76490(byte_one);
  SendByte_SN76490(byte_two);

  SendByte_SN76490(byte_three);
}

//void StartMusic(struct MIDI
void loop() {
  // put your main code here, to run repeatedly:

  StartSong(&mario_gameover); while(!PlayMusic());
  StartSong(&mario_death); while(!PlayMusic());
  StartSong(&mario_starman); while(!PlayMusic());

  SendByte_SN76490(0b11111111); // Noise channel off
  SendByte_SN76490(0b10011111); // Channel 2 off
  SendByte_SN76490(0b10111111); // Channel 1 off
  SendByte_SN76490(0b11011111); // Channel 0 off
  
  SendByte_SN76490(0b10001110);
  SendByte_SN76490(0b00001011);
  
  Serial.println("Sent");
  
  long timer=millis();

  while(millis()-timer<1000);
  
  SendByte_SN76490(0b10011111);
  Serial.println("Sent");
  
  while(1) 
  {
    if (PlayMusic()) 
    {
      Serial.println("Song ended!");
      StartSong(&mario_theme);
      //break;
    }
  }
  while(1);
}
