# SN76489_player
Arduino sketch to play pre-processed MIDI files on the SN76489 audio chip.

## Files
Example MIDI files are contained in the [MIDI folder](https://github.com/shepherdingelectrons/SN76489_player/tree/master/MIDI)
All code is in the [code folder](https://github.com/shepherdingelectrons/SN76489_player/tree/master/code)

[ProcessMidi.py](https://github.com/shepherdingelectrons/SN76489_player/blob/master/code/ProcessMidi.py) is reads the MIDI file and outputs as a slightly compressed version to a C include file. Include files are used in the Arduino sketch ([SN76489.ino](https://github.com/shepherdingelectrons/SN76489_player/blob/master/code/SN76489.ino)).

For details on how to wire the electronics, and how the library interfaces with the audio chip, see the related blog post: 

## Dependencies
The python script using the MIDI library '[python-midi](https://github.com/vishnubob/python-midi)' from [vishnubob](https://github.com/vishnubob).
The Arduino C code is standalone.
