Implemented a simple real-time streaming system using a circular/ring buffer and the libsndfile http://www.mega-nerd.com/libsndfile/ and PortAudio http://www.portaudio.com/ libraries.

The program reads a 44100Hz WAV file in frames, stores the audio into a ring buffer, and streams it to the OS' audio driver and output using a PortAudio callback.

*Includes a makefile to compile the libraries and generate the program.
