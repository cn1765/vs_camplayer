#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


enum DateType {
    RAW_DATA = 0,
    YUV_DATA,
};
enum pix_format {
    PIX_FMT_SBGGR8 = 0,
    PIX_FMT_SGBRG8,
    PIX_FMT_SGRBG8,
    PIX_FMT_SRGGB8,
    PIX_FMT_SBGGR10,
    PIX_FMT_SGBRG10,
    PIX_FMT_SGRBG10,
    PIX_FMT_SRGGB10,
    PIX_FMT_SBGGR12,
    PIX_FMT_SGBRG12,
    PIX_FMT_SGRBG12,
    PIX_FMT_SRGGB12,
    PIX_FMT_NV12,
    PIX_FMT_RGB565,
};

struct pic_info_t {
    quint32 pipe_id;
    quint32 frame_id;
    quint32 format;
    quint32 width;
    quint32 height;
    quint32 stride;
};
struct cmd_header_new_t {
	uint32_t len;
    uint32_t type; // DateType
    pic_info_t pic_info;
};

#endif // UTILS_H
