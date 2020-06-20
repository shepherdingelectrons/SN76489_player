import midi
import time

def Write_H_file(outname, name, resolution, us_pqn, data):
    milli_pqn = int(round(us_pqn/1000))
    
    f = open(outname+".h", 'w')
    # Header:
    f.write("//Auto-generated code for MIDI conversion\n")
    #f.write("static const uint16_t "+name+"_resolution="+str(resolution)+", "+name+"_milli_pqn="+str(milli_pqn)+";\n\n")

    f.write("#ifndef MIDI_STRUCTS\n");
    f.write("struct MIDIsong {\n\
  uint16_t resolution;\n\
  uint16_t milli_pqn;\n\
  uint16_t num_notes;\n\
  struct MIDInote *first_note;\n\
  struct MIDInote *curr_note;\n\
};\n\n")
    
    f.write("struct MIDInote {\n")
    f.write("uint8_t quant_reltick;\n")
    f.write("uint8_t pitch;\n")
    f.write("uint8_t duration;\n")
    f.write("uint8_t velocity;\n")
    f.write("};\n\n")

    f.write("#define MIDI_STRUCTS\n");
    f.write("#endif\n\n");

    f.write("static const struct MIDInote "+name+"_notes[] PROGMEM={\n")     
    for ni,note in enumerate(data):
        pitch, duration, velocity, reltick  = note
        quant_reltick = int(round(reltick/32.0))
        
        freq = (2**((pitch-69)/12.0))*440
        n10bit = int(round(2000000/(freq*32)))
        
        if n10bit>=1024: print "Requested frequency exceeds 10-bit representation!",pitch,freq,n10bit
        
        f.write("{"+str(quant_reltick)+","+str(pitch)+","+str(duration)+","+str(velocity)+"}")
        if ni<len(data)-1: f.write(",\n")
    f.write("\n};\n")
    f.write("static const struct MIDIsong "+ name+" = {"+str(resolution)+","+str(milli_pqn)+","+str(len(data))+",&"+name+"_notes[0],0};")
    f.close();

    #"Mario-Sheet-Music-Game-Over-Sound.mid"
    #"Mario-Sheet-Music-Overworld-Main-Theme.mid"
    #"Mario-Sheet-Music-Underworld-Theme.mid"
    #"Mario-Sheet-Music-Starman-Theme.mid"
    #"Mario-Sheet-Music-Death-Sound.mid"

output_name = "mario_theme"

midi_file = "..\MIDI\Mario-Sheet-Music-Overworld-Main-Theme.mid"
notes_per_bar = 4.0 # A constant

all_notes =[]
simulated = []

# Algorithm
# Go track by track
# Find a note on and keep a list of current notes
# Find a note off and remove from list - calculate duration and add to a
# global list of note entries [tick_start, note pitch, note duration]
# Sort by tick_start

song = midi.read_midifile(midi_file)
song.make_ticks_abs()
tracks = []

resolution = song.resolution # same as PPQ = pulses per quarter note i.e. 1024 means 1024 ticks per 'beat' (quarter note)

for track in song:
    current_notes = []
    for note in track:
        if note.name == 'Time Signature':
            pass
            #print note.numerator, note.denominator, note.metronome, note.thirtyseconds
        elif note.name == 'Set Tempo':
            #print note.bpm,note.mpqn
            bpm = note.bpm
            mpqn = note.mpqn
            #all_notes.append([note.tick,note.mpqn]) # Append a tempo change event to the note list and handle elsewhere
            
        elif note.name == 'Note On':
            current_notes.append(note)
            last_start = note.tick
        elif note.name == 'Note Off':
            current_pitches = [n.pitch for n in current_notes]
            if note.pitch in current_pitches:
                note_index = current_pitches.index(note.pitch)
                start_tick = current_notes[note_index].tick
                duration = note.tick - start_tick
                velocity = current_notes[note_index].velocity

                all_notes.append([start_tick, note.pitch, duration, velocity])
                current_notes.pop(note_index)
                
            else:
                print "ERROR: Note Off pitch not found!"
    print "************** track end *************"

all_notes.sort(key=lambda x: x[0])

print "res:",resolution # Ticks per quarter note
print "mpqn",mpqn # Microseconds Per Quarter Note
print "MILLI-seconds pqn:", mpqn/1000 # Per Quarter Note
print "1000*resolution/mpqn", 1000.0*resolution/mpqn

rel_tick = 0

for a in all_notes:
    abs_tick = a[0]
    pitch = a[1]
    duration = a[2]
    velocity = a[3]
    quant = int(round(notes_per_bar*resolution/duration)) # this expression should always be >= 1

    #print abs_tick, abs_tick-rel_tick, pitch, duration, quant
    simulated.append([pitch,quant,velocity,int(abs_tick-rel_tick)])
    rel_tick = a[0]

Write_H_file(output_name,output_name,resolution,mpqn, simulated)
