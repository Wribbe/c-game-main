#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "portaudio.h"
#include <pthread.h>

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)
typedef struct
{
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
    char message[20];
}
paTestData;

void error(PaError err);

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    for(i=0; i<framesPerBuffer; i++)
    {
        *out++ = data->sine[data->left_phase];  /* left */
        *out++ = data->sine[data->right_phase];  /* right */
        data->left_phase += 1;
        if(data->left_phase >= TABLE_SIZE) data->left_phase -= TABLE_SIZE;
        data->right_phase += 3; /* higher pitch so we can distinguish left and right. */
        if(data->right_phase >= TABLE_SIZE) data->right_phase -= TABLE_SIZE;
    }

    return paContinue;
}

/*
 * This routine is called by portaudio when playback is done.
 */
static void StreamFinished(void* userData)
{
   paTestData *data = (paTestData *) userData;
   printf("Stream Completed: %s\n", data->message);
}

/*******************************************************************/

struct thread_data {
    pthread_t thread;
    size_t index;
    PaStream * stream;
    PaStreamParameters * params;
    paTestData data;
};

void * workerfunc_threads(void * input_data)
{
    struct thread_data * thread_data = (struct thread_data *)input_data;
    printf("Hello from thread: %zu\n", thread_data->index);

    PaStream * stream = thread_data->stream;
    paTestData data = thread_data->data;

    PaError err;
    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              thread_data->params,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              patestCallback,
              &data);
    if(err != paNoError)error(err);

//    sprintf(data->message, "No Message");
    err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
    if(err != paNoError)error(err);

    err = Pa_StartStream(stream);
    if(err != paNoError)error(err);

    printf("Play for %d seconds.\n", NUM_SECONDS);
    Pa_Sleep(NUM_SECONDS * 1000);

    err = Pa_StopStream(stream);
    if(err != paNoError)error(err);

    err = Pa_CloseStream(stream);
    if(err != paNoError)error(err);

    return NULL;
}

void setup_thread_pool(size_t num_threads,
                       struct thread_data * pool,
                       PaStreamParameters * params,
                       paTestData data)
    /* Setup thread pool for all streams. */
{
    for(size_t i=0; i<num_threads; i++) {
        pool[i].index = i;
        pool[i].stream = NULL;
        pool[i].params = params;
        pool[i].data = data;
        pthread_create(&pool[i].thread, NULL, workerfunc_threads, &pool[i]);
    }
}

void error(PaError err) {
    Pa_Terminate();
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    exit(EXIT_FAILURE);
}


int main(void);
int main(void)
{
    PaStreamParameters outputParameters;
    PaError err;
    paTestData data;
    int i;

    printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

    /* initialise sinusoidal wavetable */
    for(i=0; i<TABLE_SIZE; i++)
    {
        data.sine[i] = (float) sin(((double)i/(double)TABLE_SIZE) * M_PI * 2.);
    }
    data.left_phase = data.right_phase = 0;

    err = Pa_Initialize();
    if(err != paNoError)error(err);

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default output device.\n");
      error(err);
    }
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    size_t num_threads = 10;
    struct thread_data pool[num_threads];
    setup_thread_pool(num_threads, pool, &outputParameters, data);

    for(size_t i=0; i<num_threads; i++) {
        pthread_join(pool[i].thread, NULL);
    }

    Pa_Terminate();
    printf("Test finished.\n");

    return err;
}
