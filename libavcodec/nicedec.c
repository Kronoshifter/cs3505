/*
 * NICE image format decoder
 * Copyright (c) 2005 Mans Rullgard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <inttypes.h>

#include "avcodec.h"
#include "bytestream.h"
#include "bmp.h"
#include "internal.h"
#include "msrledec.h"

static int nice_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    AVFrame *p         = data;
    int width, height;
    int x, y, ret;
    const uint8_t *buf0 = buf;

    // Check that the header is correct
    if (bytestream_get_byte(&buf) != 'N' ||
        bytestream_get_byte(&buf) != 'I' ||
	bytestream_get_byte(&buf) != 'C' ||
	bytestream_get_byte(&buf) != 'E') {
        av_log(avctx, AV_LOG_ERROR, "Ah ah ah! You didn't say the magic word. Your header is wrong.\n");
        return AVERROR_INVALIDDATA;
    }

    // Pull the width and height from the header
    width = bytestream_get_le32(&buf);
    height = bytestream_get_le32(&buf);
    
    // Set the dimensions. FFMPEG got mad at us when we took it out
    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }
    
    // Set the only supported pixel format
    avctx->pix_fmt = AV_PIX_FMT_RGB8;

    // Allocate memory
    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;
    
    // Tell FFMPEG this is a picture
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;
    
    // Set buffer position to the location of the image data
    buf   = buf0 + 12;

    // Write the image data
    for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
      {
	int index = y * width + x;
	p->data[0][index] = bytestream_get_byte(&buf);
      }
    }

    // Signal that we are finished with the frame
    *got_frame = 1;

    return buf_size;
}

AVCodec ff_nice_decoder = {
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("the nicest image compression"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .decode         = nice_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
