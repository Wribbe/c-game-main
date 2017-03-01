#include <stdlib.h>
#include <stdio.h>

#include "portaudio.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define unused(x) (void)(x)

static int callback_wav(const void * input_buffer,
                        void * output_buffer,
                        unsigned long frames_per_buffer,
                        const PaStreamCallbackTimeInfo * timeinfo,
                        PaStreamCallbackFlags status_flags,
                        void * user_data)
{
    unused(timeinfo);
    unused(status_flags);
    unused(input_buffer);

    dr_int16 ** data_pointer = (dr_int16 **)user_data;
    dr_int16 * out = (dr_int16 *)output_buffer;

    for (size_t i=0; i<frames_per_buffer; i++) {
        *out++ = **data_pointer;
        (*data_pointer)++;
        *out++ = **data_pointer;
        (*data_pointer)++;
    }

    return paContinue;
}

int main(void)
{
    const char * filename = "input/voice_16.wav";

     unsigned int channels;
     unsigned int sampleRate;
     dr_uint64 totalSampleCount;
     dr_int16 * pSampleData = drwav_open_and_read_file_s16(filename, &channels, &sampleRate, &totalSampleCount);
     if (pSampleData == NULL) {
         fprintf(stderr, "Could not open file: %s\n", filename);
         exit(EXIT_FAILURE);
     }

     PaError err = Pa_Initialize();
     if (err != paNoError) {
         fprintf(stderr, "Could not initialize PortAudio.\n");
         exit(EXIT_FAILURE);
     }

     PaStreamParameters params;
     params.device = Pa_GetDefaultOutputDevice();
     if (params.device == paNoDevice) {
         fprintf(stderr, "No default device found.\n");
         exit(EXIT_FAILURE);
     }
     params.channelCount = channels;
     params.sampleFormat = paInt16;
     params.suggestedLatency = Pa_GetDeviceInfo(params.device)->\
                               defaultHighOutputLatency;
     params.hostApiSpecificStreamInfo = NULL;

     size_t frames_per_buffer = 64;
     PaStream * stream = NULL;
     dr_int16 * data_pointer = pSampleData;
     err = Pa_OpenStream(&stream,
                         NULL,
                         &params,
                         sampleRate,
                         frames_per_buffer,
                         paClipOff,
                         callback_wav,
                         &data_pointer);

     err = Pa_StartStream(stream);
     if(err != paNoError) {
         fprintf(stderr, "Error starting stream.\n");
         exit(EXIT_FAILURE);
     }

     Pa_Sleep(5 * 1000);
     Pa_CloseStream(stream);
     Pa_Terminate();

     drwav_free(pSampleData);
}
