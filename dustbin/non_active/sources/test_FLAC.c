#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "FLAC/stream_decoder.h"
#include "portaudio.h"

#define UNUSED(x) (void)(x)

static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;

FLAC__int16 * data_buffer = NULL;
FLAC__int16 * current_write_pos = NULL;
#define FRAMES_PER_BUFFER  (128)

void metadata_callback(const FLAC__StreamDecoder * decoder,
                       const FLAC__StreamMetadata * metadata,
                       void * client_data)
{
    UNUSED(decoder);
    UNUSED(client_data);

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		total_samples = metadata->data.stream_info.total_samples;
		sample_rate = metadata->data.stream_info.sample_rate;
		channels = metadata->data.stream_info.channels;
		bps = metadata->data.stream_info.bits_per_sample;

		fprintf(stderr, "sample rate    : %u Hz\n", sample_rate);
		fprintf(stderr, "channels       : %u\n", channels);
		fprintf(stderr, "bits per sample: %u\n", bps);
		fprintf(stderr, "total samples  : %" PRIu64 "\n", total_samples);
    }
}

void error_callback(const FLAC__StreamDecoder * decoder,
                    FLAC__StreamDecoderErrorStatus status,
                    void * client_data)
{
    UNUSED(decoder);
    UNUSED(client_data);
    fprintf(stderr, "Got error callback: %s\n",\
            FLAC__StreamDecoderErrorStatusString[status]);
}

FLAC__StreamDecoderWriteStatus
write_callback(const FLAC__StreamDecoder * decoder,
               const FLAC__Frame * frame,
               const FLAC__int32 * const buffer[],
               void * client_data)
{
    UNUSED(decoder);
    for(size_t i=0; i<frame->header.blocksize; i++) {
        *current_write_pos++ = (FLAC__int16)buffer[0][i];
        *current_write_pos++ = (FLAC__int16)buffer[1][i];
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static int patestCallback(const void * inputBuffer,
                          void * outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo * timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void * userData)
{
    FLAC__int16 ** data = (FLAC__int16 *)userData;
    FLAC__int16 * out = (FLAC__int16 *)outputBuffer;

    UNUSED(timeInfo);
    UNUSED(statusFlags);
    UNUSED(inputBuffer);

    for(size_t i=0; i<framesPerBuffer; i++ )
    {
//        printf("data[0]: %d\n", (*data)[0]);
//        printf("data[1]: %d\n", (*data)[1]);
//        printf("data[2]: %d\n", (*data)[2]);
//        exit(EXIT_FAILURE);
        *out++ = (**data);
        (*data)++;
        *out++ = (**data);
        (*data)++;
    }
//    *out++ = 5;
    return paContinue;
}

int main(void)
{
    FLAC__bool ok = true;
    FLAC__StreamDecoder * decoder = FLAC__stream_decoder_new();
    FLAC__StreamDecoderInitStatus init_status;

    const char * filepath = "input/voice_16bit.flac";

    init_status = FLAC__stream_decoder_init_file(decoder,
                                                 filepath,
                                                 write_callback,
                                                 metadata_callback,
                                                 error_callback,
                                                 NULL);

	if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		fprintf(stderr, "ERROR: initializing decoder: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
		ok = false;
	}
    if(ok) {
        ok = FLAC__stream_decoder_process_until_end_of_metadata(decoder);
		fprintf(stderr, "decoding: %s\n", ok? "succeeded" : "FAILED");
		fprintf(stderr, "   state: %s\n", FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
    }
    const FLAC__uint32 total_size = (FLAC__uint32)(total_samples*\
                                                   channels*\
                                                   (bps/8));
    printf("Total size: %u bytes\n", total_size);
    size_t num_32 = total_size/sizeof(FLAC__int32);
    size_t num_16 = total_size/sizeof(FLAC__int16);
    printf("Size_uint32: %zu\n",num_32);
    data_buffer = malloc(total_size);
    current_write_pos = data_buffer;
    if (data_buffer == NULL) {
        fprintf(stderr, "Could not allocate memory for data_buffer.\n");
    }
    ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);

    FLAC__stream_decoder_delete(decoder);

    PaStreamParameters outputParameters = {0};
    PaStream * stream = NULL;
    PaError err = 0;

    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "Could not initialize PortAudio.\n");
        return(EXIT_FAILURE);
    }

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "No default output device.\n");
        return(EXIT_FAILURE);
    }
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    current_write_pos = data_buffer;
    err = Pa_OpenStream(&stream,
                        NULL, /* No input. */
                        &outputParameters,
                        sample_rate,
                        FRAMES_PER_BUFFER,
                        paClipOff, /* won't output out of range samples. */
                        patestCallback,
                        &current_write_pos);

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "Could not start stream.\n");
        return(EXIT_FAILURE);
    }

    Pa_Sleep(5*1000);
}
