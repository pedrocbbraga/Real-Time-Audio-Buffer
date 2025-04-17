/*
 * RAMTeCH Screening Test/Interview Questions for Audio Systems Software Engineer
 * Question 17
 * Simple Digital Filter
 * 
 * Author: Pedro Cajaty Barbosa Braga
 * Created on 04/16/2025
 *
 * Ring/circular buffer implementation with real-time audio
 * Run with make
 *
 */

/* ------------------------------------------------------- */

 /*
- Read WAV file
    - Use libsndfile read from question 19
    - Read a small chunk of data
- Buffer
    - While not at end of input file
        - if space in buffer
            - read N frames from PCM raw data into temp buffer
            - write frames into circular buffer
- PortAudio callback to output to OS driver
    - For each frame in framesPerBuffer
        - For each sample in channels
            - if circular buffer not empty
                - outputBuffer = circularBuffer[index]
    - Return an int
- Output filter WAV file
    - Open file
    - Read to buffer
    - Run PortAudio callback and start stream
    - Close input + output files
*/

/* ------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include <portaudio.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define FRAMES 256
#define BUFFER_SIZE 2048

float circBuffer[BUFFER_SIZE];
int writeIdx = 0;
int readIdx = 0;
int eof = 0;

typedef struct
{
    SNDFILE* file;
    SF_INFO originalInfo;
} WAV;

WAV openAudioFile(const char* fileName) {
    WAV wav;
    wav.originalInfo.format = 0;
    wav.file = sf_open(fileName, SFM_READ, &wav.originalInfo);
    if (!wav.file)
    {
        fprintf(stderr, "Couldn't open file\n");
        exit(1);
    }
    return wav;
}

int circBufferCanWrite()
{
    return ((writeIdx + 1) % BUFFER_SIZE) != readIdx;
}

int circBufferHasData()
{
    // int hasData = readIdx != writeIdx;
    // printf("has data: %d\n", hasData);
    return readIdx != writeIdx;
}

void populateCircularBuffer(WAV* wav)
{
    float temp[FRAMES * CHANNELS];
    sf_count_t readFrames = sf_readf_float(wav->file, temp, FRAMES);

    if (readFrames == 0)
    {
        // printf("WE ARE HERE\n");
        eof = 1;
        return;
    }
    
    for(int i=0; i < readFrames * CHANNELS; i++)
    {
        circBuffer[writeIdx] = temp[i];
        writeIdx = (writeIdx + 1) % BUFFER_SIZE;
    }
}

static int paStreamCallback(const void* inputBuffer, void* outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void* userData)
{
    // Bypass compiler warnings due to unused variables
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;
    
    WAV* wav = (WAV*)userData;
    float* out = (float*) outputBuffer;

    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        for (int j = 0; j < CHANNELS; j++)
        {
            int idx = i * CHANNELS + j;

            if (circBufferHasData())
            {
                out[idx] = circBuffer[readIdx];
                readIdx = (readIdx + 1) % BUFFER_SIZE;
                // printf("Read Index: %d\n", readIdx);
            }
            else
            {
                out[idx] = 0.0f;
            }
        }
    }

    if (circBufferCanWrite())
    {
        populateCircularBuffer(wav);
    }

    if (eof && !circBufferHasData())
    {
        // printf("WE ARE HERE\n");
        return paComplete;
    }
    return paContinue;
}

int main()
{   
    char fileName[512];
    printf("Enter WAV file path to stream: ");
    if (fgets(fileName, sizeof(fileName), stdin) == NULL)
    {
        fprintf(stderr, "Couldn't read file\n");
        return 1;
    }
    fileName[strcspn(fileName, "\n")] = '\0';
    WAV wav = openAudioFile(fileName);
    // printf("OPENED FILE\n");

    if (wav.originalInfo.channels != CHANNELS || wav.originalInfo.samplerate != SAMPLE_RATE)
    {
        printf("Invalid sample rate and # of channels. Requires: %d Hz, %d channels.\n", SAMPLE_RATE, CHANNELS);
        sf_close(wav.file);
        return 1;
    }

    // Attempt at pre-populating the circular buffer to avoid the initial spike/glitch, needs improvement
    for (int i = 0; i < 4 && !eof; ++i)
    {
        populateCircularBuffer(&wav);
    }    

    Pa_Initialize();
    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 0, CHANNELS, paFloat32, SAMPLE_RATE, FRAMES, paStreamCallback, &wav);
    Pa_StartStream(stream);

    printf("Streaming. Program will close at EOF.\n");
    while (Pa_IsStreamActive(stream) == 1)
    {
        Pa_Sleep(100);
    }

    printf("Done.\n");
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    sf_close(wav.file);

    return 0;
}
