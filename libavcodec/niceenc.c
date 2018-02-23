/*
 * NICE image format encoder
 * Copyright (c) 2006, 2007 Michel Bardiaux
 * Copyright (c) 2009 Daniel Verkamp <daniel at drv.nu>
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

#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "bmp.h"
#include "internal.h"

static av_cold int nice_encode_init(AVCodecContext *avctx){
    
  // Set bits per pixel
    avctx->bits_per_coded_sample = 8;

    return 0;
}

static int nice_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    const AVFrame * const pixel_data = pict;
    int total_bytes, x, y, header_size, ret;
    uint8_t *buf;
    
    // Let context know to display as a picture instead of a video
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    avctx->coded_frame->key_frame = 1;

    // Header size will always be 12 bytes
    header_size = 12;

    // Calculate the number of bytes in the file
    total_bytes = avctx->width * avctx->height + header_size;

    // Allocate memory for image file, if it fails end here
    if ((ret = ff_alloc_packet2(avctx, pkt, total_bytes, 0)) < 0)
        return ret;

    // Set starting point for data buffer
    buf = pkt->data;

    // Write header
    bytestream_put_byte(&buf, 'N');                   
    bytestream_put_byte(&buf, 'I');                   
    bytestream_put_byte(&buf, 'C');                   
    bytestream_put_byte(&buf, 'E');                   
    bytestream_put_le32(&buf, avctx->width);
    bytestream_put_le32(&buf, avctx->height);
    
    // Set buffer position for writing data
    buf = pkt->data + header_size;
    
    // Write image data
    for (y = 0; y < avctx->height; y++)
    {
      for (x = 0; x < avctx->width; x++)
      {
	int index = y * avctx->width + x;
	bytestream_put_byte(&buf, pixel_data->data[0][index]);
      }
    }

    // Signal that we got the packet and wrote the file
    *got_packet = 1;

    return 0;
}

AVCodec ff_nice_encoder = {
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("the nicest image compression"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .init           = nice_encode_init,
    .encode2        = nice_encode_frame,
    .pix_fmts       = (const enum AVPixelFormat[]){
        AV_PIX_FMT_RGB8,
        AV_PIX_FMT_NONE
    },
};
