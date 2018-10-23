/**
 * \file posix-stream-sdl.c
 *
 * \brief Example of using the library to stream instrument data to a GUI
 * application in a POSIX environment.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for PATH_MAX in limits.h. */
#define _POSIX_C_SOURCE 199309L

/* Required for errno. */
#include <errno.h>
/* Required for open. */
#include <fcntl.h>
/* Required for PATH_MAX. */
#include <limits.h>
/* Required for isnan. */
#include <math.h>
/* Required for SDL_*. */
#include <SDL2/SDL.h>
/* Required for fprintf. */
#include <stdio.h>
/* Required for memcpy, strerror. */
#include <string.h>
/* Required for open. */
#include <sys/stat.h>
/* Required for close, read, write. */
#include <unistd.h>

#include "posix-shared.h"

#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))

#define SAMPLE_SIZE (60 * 12)

#define VER_PAD 32

static int PLOT_COLORS[][3] = {
    {0xFF, 0x33, 0x33},
    {0x33, 0x33, 0xFF},
    {0x00, 0x80, 0x80},
    {0xFF, 0x80, 0x80},
    {0x00, 0x00, 0x80},
    {0x00, 0x80, 0x00},
    {0x80, 0x00, 0x80},
    {0x00, 0x80, 0x80},
    {0x80, 0x80, 0x80},
    {0x80, 0x80, 0x00},
    {0x80, 0x00, 0x80},
    {0x33, 0xFF, 0x33},
    {0xFF, 0x55, 0xFF},
    {0x80, 0x00, 0x00},
    {0xc0, 0x00, 0x00},
    {0x00, 0x00, 0xC0},
    {0x00, 0xC0, 0x00},
    {0x40, 0x40, 0x40},
    {0xFF, 0x40, 0x40},
    {0x40, 0x40, 0xFF},
    {0x40, 0xFF, 0x40},
    {0xFF, 0x40, 0xFF}
};
#define PLOT_COLORS_LEN (sizeof(PLOT_COLORS) / sizeof(PLOT_COLORS[0]))

static RBRInstrumentSample callbackSample;
static RBRInstrumentSample *samples = NULL;
static SDL_Point *samplePoints[RBRINSTRUMENT_CHANNEL_MAX] = {
    NULL
};
static int sampleCount = 0;
static int samplePointChannelCount = 0;
static int samplePointCount = 0;

static SDL_Window *window;
static SDL_Renderer *renderer;

RBRInstrumentError instrumentSample(
    const struct RBRInstrument *instrument,
    const struct RBRInstrumentSample *const sample)
{
    /* Unused. */
    instrument = instrument;

    if (sampleCount == SAMPLE_SIZE)
    {
        memmove(&samples[0],
                &samples[1],
                (sampleCount-- *sizeof(RBRInstrumentSample)));
    }

    memcpy(&samples[sampleCount++], sample, sizeof(RBRInstrumentSample));

    return RBRINSTRUMENT_SUCCESS;
}

static void recalculatePoints()
{
    int width;
    int height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    RBRInstrumentDateTime minTime = samples[0].timestamp,
                          maxTime = samples[sampleCount - 1].timestamp;
    RBRInstrumentDateTime duration = maxTime - minTime;
    double horScale = ((double) width) / duration;

    samplePointChannelCount = RBRINSTRUMENT_CHANNEL_MAX;
    double minVal[RBRINSTRUMENT_CHANNEL_MAX] = {
        DBL_MAX
    };
    double maxVal[RBRINSTRUMENT_CHANNEL_MAX] = {
        DBL_MIN
    };
    for (int i = 0; i < sampleCount; i++)
    {
        samplePointChannelCount = MIN(samples[i].channels,
                                      samplePointChannelCount);
        for (int channel = 0; channel < samplePointChannelCount; channel++)
        {
            minVal[channel] = MIN(samples[i].readings[channel],
                                  minVal[channel]);
            maxVal[channel] = MAX(samples[i].readings[channel],
                                  maxVal[channel]);
        }
    }
    double verScale[RBRINSTRUMENT_CHANNEL_MAX];
    for (int channel = 0; channel < samplePointChannelCount; channel++)
    {
        verScale[channel] = ((double) (height - 2 * VER_PAD)) / (maxVal[channel] - minVal[channel]);
    }

    for (int i = 0; i < sampleCount; i++)
    {
        RBRInstrumentDateTime timestamp = samples[i].timestamp - minTime;
        for (int channel = 0; channel < samplePointChannelCount; channel++)
        {
            double value = samples[i].readings[channel] - minVal[channel];
            samplePoints[channel][i].x = timestamp * horScale;
            samplePoints[channel][i].y = height - (value * verScale[channel]) - VER_PAD;
        }
    }
    samplePointCount = sampleCount;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentError error;
    RBRInstrument *instrument = NULL;

    if ((samples = malloc(sizeof(RBRInstrumentSample) * SAMPLE_SIZE)) == NULL)
    {
        fprintf(stderr,
                "%s: Failed to allocate sample buffer!\n",
                programName);
        return EXIT_FAILURE;
    }

    for (int channel = 0; channel < RBRINSTRUMENT_CHANNEL_MAX; channel++)
    {
        if ((samplePoints[channel] = malloc(sizeof(SDL_Point) * SAMPLE_SIZE))
            == NULL)
        {
            fprintf(stderr,
                    "%s: Failed to allocate point buffer %d!\n",
                    programName,
                    channel + 1);
            return EXIT_FAILURE;
        }
    }

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0)
    {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n",
                programName,
                strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRINSTRUMENT_LIB_NAME,
            RBRINSTRUMENT_LIB_VERSION,
            RBRINSTRUMENT_LIB_BUILD_DATE);

    RBRInstrumentCallbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &callbackSample
    };

    if ((error = RBRInstrument_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRINSTRUMENT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentError_name(error));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRInstrumentLink link;
    RBRInstrument_getLink(instrument, &link);

    switch (link)
    {
    case RBRINSTRUMENT_LINK_USB:
        RBRInstrument_setUSBStreamingState(instrument, true);
        break;
    case RBRINSTRUMENT_LINK_SERIAL:
    case RBRINSTRUMENT_LINK_WIFI:
        RBRInstrument_setSerialStreamingState(instrument, true);
        break;
    default:
        fprintf(stderr,
                "%s: I don't know how I'm connected to the instrument, so I"
                " can't enable streaming. Giving up.\n",
                programName);
        goto instrumentCleanup;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "%s: Failed to initialize SDL!\n", programName);
        goto instrumentCleanup;
    }

    window = SDL_CreateWindow(
        "POSIX Streaming Example",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL)
    {
        fprintf(stderr, "%s: Failed to initialize SDL window!\n", programName);
        goto instrumentCleanup;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        fprintf(stderr,
                "%s: Failed to initialize SDL window/renderer!\n",
                programName);
        goto instrumentCleanup;
    }

    SDL_bool done = SDL_FALSE;

    RBRInstrumentError err;
    while (!done)
    {
        if ((err = RBRInstrument_readSample(instrument)) != RBRINSTRUMENT_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Error: %s\n",
                    programName,
                    RBRInstrumentError_name(err));
        }

        /* We've received new samples; recalculate all the points. */
        if (sampleCount >= 2 &&
            (samplePointCount < sampleCount || sampleCount == SAMPLE_SIZE))
        {
            recalculatePoints();
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for (int channel = 0; channel < samplePointChannelCount; channel++)
        {
            SDL_SetRenderDrawColor(renderer,
                                   PLOT_COLORS[channel % PLOT_COLORS_LEN][0],
                                   PLOT_COLORS[channel % PLOT_COLORS_LEN][1],
                                   PLOT_COLORS[channel % PLOT_COLORS_LEN][2],
                                   SDL_ALPHA_OPAQUE);
            SDL_RenderDrawLines(renderer,
                                samplePoints[channel],
                                samplePointCount);
        }
        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                done = SDL_TRUE;
            }
        }
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
        SDL_DestroyWindow(window);
    }

instrumentCleanup:
    RBRInstrument_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
