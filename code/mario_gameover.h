//Auto-generated code for MIDI conversion
#ifndef MIDI_STRUCTS
struct MIDIsong {
  uint16_t resolution;
  uint16_t milli_pqn;
  uint16_t num_notes;
  struct MIDInote *first_note;
  struct MIDInote *curr_note;
};

struct MIDInote {
uint8_t quant_reltick;
uint8_t pitch;
uint8_t duration;
uint8_t velocity;
};

#define MIDI_STRUCTS
#endif

static const struct MIDInote mario_gameover_notes[] PROGMEM={
{0,64,4,61},
{0,72,4,75},
{0,55,4,69},
{96,60,4,56},
{0,67,4,70},
{0,52,4,64},
{96,55,2,56},
{0,64,2,70},
{0,48,2,64},
{63,69,3,69},
{0,65,1,64},
{0,53,1,64},
{43,71,3,64},
{43,69,3,64},
{42,68,2,69},
{0,65,1,64},
{0,49,1,64},
{65,70,2,64},
{63,68,2,69},
{65,64,4,64},
{0,67,1,64},
{0,48,1,64},
{32,62,4,64},
{32,64,1,64}
};
static const struct MIDIsong mario_gameover = {1024,150,24,&mario_gameover_notes[0],0};