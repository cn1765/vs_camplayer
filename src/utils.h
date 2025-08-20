#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cmd_header_new_t {
	quint32 header_start;
    quint32 header_check1;
    quint32 header_check2;
    quint32 header_end;
    quint32 header_crc;
	quint32 len;
	quint32 type; // DateType
    quint32 format; // RAW_BIT or YUV_TYEP
	union {
		struct {
			quint32 width;
			quint32 height;
			quint32 stride;
			quint32 frame_plane; // sensor_mode
			quint32 code_type; //  VIDEO_TYPE
			quint32 pipe_info;
		} pic_i;
		struct {
			quint32 cmd_id;
		    quint32 command_id;
			quint32 value;
		} api_rw;
		struct {
		    quint32 id;
			quint32 type;
			quint32 size;
			quint32 width;
			quint32 rows;
			quint32 cols;
		} calib_rw;
		struct {
			quint32 offset;
			quint32 size;
			quint32 type; // mask or not
		} reg_rw;
	} metadata;
	union {
		struct {
    		quint32 pipe_id;
			quint32 chn_id;
			quint32 frame_id;
		} r_i;
		struct {
			quint32 pipe_id;
    		quint32 direction;
			quint32 count_id;
		} s_i;
	} packinfo;
    quint32 chip_version;
    quint32 plugin_id;
    quint32 reserved2;
};

struct param_buf_t {
	quint32 param_id;
	quint32 param_data;
};

struct tranfer_info_t {
	quint8 tcp_open;
	quint8 raw_enable;
	quint8 raw_serial_num;
	quint8 yuv_enable;
	quint8 yuv_serial_num;
	quint8 jepg_enable;
	quint8 video_enable;
	quint8 video_code;
	quint16 bit_stream;
	quint16 fream_interval;
	quint16 pipe_line;
	quint16 channel_id;
	struct param_buf_t video_cfg;
};


static struct cmd_header_new_t start_cmd_header_new_t = {
	0,
	0,
	0,
	0,
	0,
	24,
	13,
	0,
	2353168,
	0,
	11054412,
	0,
	11240576,
	0,
	11347990,
	0,
	2353232,
	0,
	0,
	0,
};

static struct cmd_header_new_t stop_cmd_header_new_t = {
	0,
	0,
	0,
	0,
	0,
	24,
	13,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1743172308,
	32764,
	0,
	0,
	1,
	0,
};

static struct tranfer_info_t start_transfer_info = {
	1,
	1,
	0,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

static struct tranfer_info_t stop_transfer_info = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};
#endif // UTILS_H
