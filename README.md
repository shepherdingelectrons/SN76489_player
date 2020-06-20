# SN76489_player
Arduino sketch to play pre-processed MIDI files on the SN76489 chip

Example MIDI files are contained in the MIDI folder
All code is in the code folder

ProcessMidi.py is reads the MIDI file and outputs as a slightly compressed version to a C include file. Include files are used in the Arduino sketch (SN76489.ino).

For details on how to wire the electronics, and how the library interfaces with the audio chip, see the related blog post: 
