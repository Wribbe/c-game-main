#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vorbis/codec.h"
#include "vorbisfile.h"
#include "portaudio.h"

#define unused(x) (void)(x)

char pcmout[4096] = {0};

void print_comment(OggVorbis_File * vf)
{
    char ** ptr = ov_comment(vf, -1)->user_comments;
    vorbis_info * vi = ov_info(vf, -1);
    while(*ptr) {
        fprintf(stderr, "%s\n", *ptr++);
    }
    fprintf(stderr, "\nBitstream is %d channel, %ldHz\n",
            vi->channels,
            vi->rate);
    fprintf(stderr, "\nDecoded length: %ld samples\n",
            (long)ov_pcm_total(vf, -1));
    fprintf(stderr, "Encoded by: %s\n\n", ov_comment(vf, -1)->vendor);
    fprintf(stderr, "upper bitrate: %ld\n", vi->bitrate_upper);
    fprintf(stderr, "nominal bitrate: %ld\n", vi->bitrate_nominal);
    fprintf(stderr, "lower bitrate: %ld\n", vi->bitrate_lower);
}

int little_endian = 0;
int sample_16_bit = 2;
int sample_signed = 1;

static int ogg_callback(const void * inputBuffer,
                        void * outputBuffer,
                        unsigned long frames_per_buffer,
                        const PaStreamCallbackTimeInfo * timeinfo,
                        PaStreamCallbackFlags statusFlags,
                        void * user_data)
{
    int16_t ** data_pointer = (int16_t **)(user_data);
    int16_t * out = (int16_t *)outputBuffer;

    unused(timeinfo);
    unused(statusFlags);
    unused(inputBuffer);

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
    OggVorbis_File vf = {0};
    int eof = 0;
    int current_section = 0;

    const char * filepath = "input/voice.ogg";
    ov_fopen(filepath, &vf);
    print_comment(&vf);
    vorbis_info * vi = ov_info(&vf, -1);

    long header_size = (long)ov_pcm_total(&vf, -1);
    size_t total_size = header_size*sample_16_bit*vi->channels;

    int16_t * data_buffer = malloc(total_size*sizeof(int16_t));
    if(!data_buffer) {
        fprintf(stderr, "Could not allocate enough data for data_buffer.\n");
        exit(EXIT_FAILURE);
    }

    int16_t * buffer_pointer = data_buffer;
    // Vorbis-read loop.
    while(!eof) {
        long ret = ov_read(&vf,
                           pcmout,
                           sizeof(pcmout),
                           little_endian,
                           sample_16_bit,
                           sample_signed,
                           &current_section);
        if (ret == 0) {
            eof=1;
        }
        // Copy read data into buffer.
        memcpy(buffer_pointer, pcmout, ret);
        // Advance pointer according to sample size.
        buffer_pointer += ret/sample_16_bit;
    }

    /* Initialize PortAudio. */
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "Could not initialize PortAudio.\n");
        exit(EXIT_FAILURE);
    }

    /* Set up output device and output parameters. */
    PaStreamParameters out_params = {0};
    out_params.device = Pa_GetDefaultOutputDevice();
    if (out_params.device == paNoDevice) {
        fprintf(stderr, "Could not find a default device.\n");
        exit(EXIT_FAILURE);
    }

    out_params.channelCount = vi->channels;
    out_params.sampleFormat = paInt16;
    const PaDeviceInfo * devinfo = Pa_GetDeviceInfo(out_params.device);
    out_params.suggestedLatency = devinfo->defaultHighOutputLatency;
    out_params.hostApiSpecificStreamInfo = NULL;

    /* Setup stream. */
    PaStream * stream = NULL;
    size_t frames_per_buffer = 64;
    buffer_pointer = data_buffer;
    err = Pa_OpenStream(&stream,
                        NULL, // No input.
                        &out_params,
                        vi->rate,
                        frames_per_buffer,
                        paClipOff,
                        ogg_callback,
                        &buffer_pointer);
    if (err != paNoError) {
        fprintf(stderr, "Could not setup stream.\n");
        exit(EXIT_FAILURE);
    }

    err = Pa_StartStream(stream);
    Pa_Sleep(5 * 1000);
}
