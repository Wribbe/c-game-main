#include <stdlib.h>
#include <stdio.h>
#include "portaudio.h"
#include <math.h>

#define unused(x) (void)x
#define SAMPLE_RATE 44100
#define TABLE_SIZE 200

#ifndef M_PI
#define M_PI 3.14159265
#endif

typedef struct  {
    float sine[TABLE_SIZE];
    float left_phase;
    float right_phase;
} paTestData;

/* This routine will be called by the PortAudio engine when audio is needed. It
 * may be called at interrupt level on some machines, so don't do anything that
 * could mess up the system, like calling malloc() or free().
 */

static int patestCallback(const void * inputBuffer,
                          void * outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo * timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void * userData ) {
    /* Cast data passed through stream to our structure. */
    paTestData * data = (paTestData * )userData;
    float * out = (float *)outputBuffer;
    unsigned int i = 0;
    unused(inputBuffer); // Prevent unused warning.
    unused(statusFlags);
    unused(timeInfo);

    for (i=0; i<framesPerBuffer; i++) {
        *out++ = data->left_phase;      // left.
        *out++ = data->right_phase;     // right.

        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        data->left_phase += 0.01f;
        /* When signal reaches top, drop it back down. */
        if (data->left_phase >= 1.0f) {
            data->left_phase -= 2.0f;
        }
        /* Use higher pitch on right channel. */
        data->right_phase += 0.03f;
        if (data->right_phase >= 1.0f) {
            data->right_phase -= 2.0f;
        }
    }
    return 0;
}

int main(void) {

    // Initialize PortAudio.
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "[!] PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    printf("PortAudio initialized without errors.\n");

    // Set up default data.
    int output_channels = 2;
    int frames_per_buffer = 256;
    static paTestData data;

    PaStream * stream;
    PaStreamParameters outputParameters;

    // Set default device. (don't forget this.)
    outputParameters.device = Pa_GetDefaultOutputDevice();

    outputParameters.channelCount = output_channels;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    /* Open an audio I/O stream. */
    err = Pa_OpenStream(
                        &stream,
                        NULL,
                        &outputParameters,
                        SAMPLE_RATE,
                        frames_per_buffer,
                        paClipOff,
                        patestCallback,
                        &data
                       );
    if (err != paNoError) {
        fprintf(stderr, "[!] PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    /* Construct sin wave. */
    for(int i=0; i<TABLE_SIZE; i++) {
        data.sine[i] = (float) sin(((double)i/(double)TABLE_SIZE) * M_PI * 2.0);
    }
    /* Initialize left and right phases. */
    data.left_phase = 0;
    data.right_phase = 0;

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "[!] PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    /* Sleep so that there is time to play the stream. */
    printf("Play for %d seconds.\n", 10);
    Pa_Sleep(10 * 1000);

    /* Stop stream. */
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "[!] PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    /* Close stream. */
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "[!] PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    /* Terminate stream. */
    err = Pa_Terminate();
    if (err != paNoError) {
        fprintf(stderr, "[!] PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    printf("PortAudio terminated without errors.\n");

    return 0;
}
