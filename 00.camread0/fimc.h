#ifndef FIMC_H_
#define FIMC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>
#include <linux/videodev2.h>

#define CAMERA_DEVICE 	"/dev/video1"
#define CAPTURE_FILE 	"video.h264"

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define VIDEO_FORMAT V4L2_PIX_FMT_H264
#define BUFFER_COUNT 4

typedef struct VideoBuffer {
    void   *start;
    size_t  length;
} VideoBuffer;


#endif /* FIMC_H_ */

