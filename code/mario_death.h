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

static const struct MIDInote mario_death_notes[] PROGMEM={
{0,72,15,69},
{9,73,17,64},
{8,74,21,67},
{111,67,4,61},
{0,71,4,75},
{0,55,4,69},
{33,74,4,56},
{0,77,4,70},
{64,74,4,56},
{0,77,4,66},
{0,55,4,64},
{32,74,6,61},
{0,77,6,75},
{0,55,6,69},
{43,72,6,56},
{0,76,6,70},
{0,57,6,64},
{43,71,6,56},
{0,74,6,70},
{0,59,6,64},
{42,67,4,61},
{0,72,4,75},
{0,60,4,69},
{33,64,4,64},
{32,55,4,64},
{32,64,4,64},
{32,60,4,69},
{0,48,4,69}
};
static const struct MIDIsong mario_death = {1024,150,28,&mario_death_notes[0],0};