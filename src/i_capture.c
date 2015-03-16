//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2015 Fabian Greffrath
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Video capturing stuff.
//

#include "SDL.h"

#include "config.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_system.h"

// This is our global "-viddump" feature indicator ;)
void *I_VideoCapture = NULL;

#if defined (HAVE_LIBAVUTIL) && \
    defined (HAVE_LIBAVCODEC) && \
    defined (HAVE_LIBAVFORMAT) && \
    defined (HAVE_LIBSWSCALE)

#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

static AVCodecContext *context = NULL;
static AVCodec *codec;
static AVFormatContext *oc = NULL;
static AVOutputFormat *fmt;
static AVStream *st;
static AVFrame *frame;
static AVPacket pkt;
static struct SwsContext *swscontext = NULL;

// Our own palette
static uint32_t avpalette[256] = {0xff};

// Our line pitch
static int stride[] = {SCREENWIDTH, 0, 0, 0};

// Our raw video encoding data, data[0] will point to I_VideoBuffer later
static uint8_t *data[] = {NULL, (uint8_t *) avpalette, NULL, NULL};

void I_InitCapture (const char *filename)
{
    int ret;

    // Make sure this function in called only once
    if (context || swscontext)
    {
	I_Error("I_InitCapture: Internal error!");
    }

    // Initialize all available formats and codecs
    av_register_all();
    avcodec_register_all();

    // Guess container format based on the file name passed on the
    // command line, fall back to Matroska if ambigious
    fmt = av_guess_format(NULL, filename, NULL);
    if (!fmt)
    {
	fprintf(stderr, "I_InitCapture: Failed to deduce output format from "
	                "file extension, guessing Matroska.\n");
	fmt = av_guess_format("matroska", NULL, NULL);
    }
    if (!fmt)
    {
	I_Error("I_InitCapture: Failed to set output format!");
    }

    // Prepare avformat context
    oc = avformat_alloc_context();
    if (!oc)
    {
	I_Error("I_InitCapture: Failed to allocate avformat context!");
    }
    oc->oformat = fmt;
    snprintf(oc->filename, sizeof(oc->filename), "%s", filename);

    // Find default video encoder for the chosen container format
    codec = avcodec_find_encoder(fmt->video_codec);
    if (!codec)
    {
	I_Error("I_InitCapture: Failed to find video codec!");
    }

    // Prepare avformat video stream
    st = avformat_new_stream(oc, codec);
    if (!st)
    {
	I_Error("I_InitCapture: Failed to allocate avformat stream!");
    }

    // Set stream frame rate to 35 fps
    st->time_base = (AVRational){1, TICRATE};

    // Point avcodec context to stream's codec context
    context = st->codec;
    context->width = SCREENWIDTH;
    context->height = SCREENHEIGHT;
    context->time_base = st->time_base;

    // This will almost certainly return AV_PIX_FMT_YUV420P
    context->pix_fmt = context->get_format(context, codec->pix_fmts);

    // Some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
	context->flags |= CODEC_FLAG_GLOBAL_HEADER;

    // Open the video codec
    ret = avcodec_open2(context, codec, NULL);
    if (ret < 0)
    {
	I_Error("I_InitCapture: Failed to open video codec!");
    }

    // Prepare a video frame and allocate a frame buffer for the
    // picture in YUV420 format
    frame = av_frame_alloc();
    if (!frame)
    {
	I_Error("I_InitCapture: Failed to allocate video encoding frame!");
    }
    ret = av_image_alloc(frame->data, frame->linesize,
                         context->width, context->height, context->pix_fmt,
                         32);
    if (ret < 0)
    {
	I_Error("I_InitCapture: Failed to allocate raw picture buffer!");
    }
    frame->format = context->pix_fmt;
    frame->width = context->width;
    frame->height = context->height;
    frame->pts = 0;

    // Open the output file for writing
    ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
	I_Error("I_InitCapture: Failed to open file %s %d!", filename, ret);
    }

    // Print some information about the chosen container format and video codec
    av_dump_format(oc, 0, filename, 1);

    // Write the container header, if any
    avformat_write_header(oc, NULL);

    // Prepare software scaling context for the 8-bit->YUV420 conversion
    swscontext = sws_getContext(context->width, context->height, AV_PIX_FMT_PAL8,
                                context->width, context->height, context->pix_fmt,
                                0, NULL, NULL, NULL);
    if (!swscontext)
    {
	I_Error("I_InitCapture: Failed to allocate software scaling context!");
    }

    // Point this to something non-NULL to indicate
    // that the "-viddump" feature is enabled
    I_VideoCapture = context;
}

void I_CaptureEncode (void)
{
    int ret, got_packet;

    // Start with an empty packet
    memset(&pkt, 0, sizeof(pkt));
    av_init_packet(&pkt);

    // Make sure this is only called after initialization
    if (!context || !swscontext)
    {
	I_Error("I_CaptureEncode: Internal error!");
    }

    // Scale our paletted 8-bit frame buffer into the YUV420 buffer
    ret = sws_scale(swscontext,
                    (const uint8_t * const*) data, stride, 0,
                    frame->height, frame->data, frame->linesize);
    if (ret < 0)
    {
	I_Error("I_CaptureEncode: Failed to scale raw picture buffer!");
    }

    // Encode the video frame
    ret = avcodec_encode_video2(context, &pkt, frame, &got_packet);
    if (ret < 0)
    {
	I_Error("I_CaptureEncode: Failed to encode video frame!");
    }

    if (got_packet)
    {
	// Synchronize the video stream with the container
	pkt.stream_index = st->index;
	av_packet_rescale_ts(&pkt, context->time_base, st->time_base);

	// Write to output file
	ret = av_interleaved_write_frame(oc, &pkt);
    }

    if (ret < 0)
    {
	I_Error("Failed to write video frame!");
    }

    // Maintain monotonic packet time stamps
    frame->pts++;
}

void I_SetCapturePalette (SDL_Color *palette)
{
    int i;

    // Unfortunately, we need to maintain our own palette here,
    // because SDL stores its in SDL_Color format which has RGBA
    // byte order whereas avcodec expects it to be in ARGB order.
    for (i = 0; i < 256; i++)
    {
	avpalette[i] = (0xff << 24) |
	               (palette[i].r << 16) |
	               (palette[i].g << 8) |
	               (palette[i].b);
    }

    // Initialize here, because I_VideoBuffer isn't known earlier
    if (!data[0])
    {
	data[0] = I_VideoBuffer;
    }
}

void I_QuitCapture (void)
{
    // Make sure this is only called after initialization
    if (!context || !swscontext)
    {
	I_Error("I_QuitCapture: Internal error!");
    }

    // Encode the last remaining frame, if any
    I_CaptureEncode();

    // Write the format trailer to the output file, if any
    av_write_trailer(oc);

    avcodec_close(st->codec);
    av_frame_free(&frame);

    // Close the output file
    avio_close(oc->pb);
    avformat_free_context(oc);

    sws_freeContext(swscontext);
}

#else

void I_InitCapture (const char *filename)
{
    I_Error("I_InitCapture: The \"-viddump\" feature is not available,\n"
            "please rebuild %s with libav support.",
            PACKAGE_NAME);
}
void I_CaptureEncode (void) {};
void I_SetCapturePalette (SDL_Color *palette) {};
void I_QuitCapture (void) {};

#endif
