#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <linux/videodev2.h>


#define VIDEO_WIDTH	640
#define VIDEO_HEIGHT	480
#define VIDEO_FORMAT	V4L2_PIX_FMT_H264
#define BUFFER_COUNT	4


struct buffer
{
	void* start;
	unsigned int length;
};

int hexDump(char* buf, int buf_len)
{
	int i = 0;
	for(i = 0; i < buf_len; i++)
	{
		printf("%02x ", buf[i]);
		if((i + 15)%16 == 0)
		{
			printf("\n");
		}
	}

	printf("\n");

	return 0;
}

int process_frame(void* frame, int frame_len)
{
	printf("frame_len:%d\n", frame_len);
	return 0;
}

int openCam(char* camName)
{
	int fd = open(camName, O_RDWR);
	if(fd < 0)
	{
		return -1;
	}

	return fd;
}

int closeCam(int fd)
{
	if(fd != -1)
	{
		return close(fd);
	}
	return -1;
}

int getCamInfo(int fd)
{
	int ret = -1;
	struct v4l2_capability cap;
	memset(&cap, 0, sizeof(cap));
	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0)
	{
		printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
		perror("VIDIOC_QUERYCAP");
		return ret;
	}
	
	printf("Camera Capability Informations:\n");
	printf("\tDriver Name:%s\n", cap.driver);
	printf("\tCard Name:%s\n", cap.card);
	printf("\tBuf info:%s\n", cap.bus_info);
	printf("\tDriver Version:%u.%u.%u\n", (cap.version>>16)&0xff, (cap.version>>8)&0xff, cap.version&0xff);
	printf("\tCapabilities:0x%08x\n", cap.capabilities);
	return ret;
}

int dumpCamFmt(struct v4l2_format* pFmt)
{
	if(NULL == pFmt)
	{
		return -1;
	}

	char fmtStr[8] = {0};
	memcpy(fmtStr, &pFmt->fmt.pix.pixelformat, 4);

	printf("Stream Format Informations:\n");
	printf("\ttype:%d\n", pFmt->type);
	printf("\twidth:%d\n", pFmt->fmt.pix.width);
	printf("\theight:%d\n", pFmt->fmt.pix.height);
	printf("\tpixelformat:%s\n", fmtStr);
	printf("\tfield:%d\n", pFmt->fmt.pix.field);
	printf("\tbytesPerLine:%d\n", pFmt->fmt.pix.bytesperline);
	printf("\tsizeimage:%d\n", pFmt->fmt.pix.sizeimage);
	printf("\tcolorspace:%d\n", pFmt->fmt.pix.colorspace);
	printf("\tpriv:%d\n", pFmt->fmt.pix.priv);
	printf("\traw_data:%s\n", pFmt->fmt.raw_data);

	return 0;
}

int loadCamFmt(struct v4l2_format *pFmt)
{
	if(NULL == pFmt)
	{
		return -1;
	}

	memset(pFmt, 0, sizeof(struct v4l2_format));
	pFmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pFmt->fmt.pix.width = VIDEO_WIDTH;
	pFmt->fmt.pix.height = VIDEO_HEIGHT;
	pFmt->fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
	pFmt->fmt.pix.field = V4L2_FIELD_INTERLACED;

	return 0;
}

int getCamFmt(int fd)
{
	printf("%s\n", __func__);
	int ret = -1;
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	ret = ioctl(fd, VIDIOC_G_FMT, &fmt);
	if(ret < 0)
	{
		printf("VIDIOC_G_FMT failed (%d)\n", ret);
		perror("VIDIOC_G_FMT");
		return ret;
	}

	dumpCamFmt(&fmt);
	return ret;
}

int setCamFmt(int fd, struct v4l2_format *pFmt)
{
	int ret = -1;

	if(NULL == pFmt)
	{
		return -1;
	}

	ret = ioctl(fd, VIDIOC_S_FMT, pFmt);
	if(ret < 0)
	{
		printf("VIDIOC_S_FMT failed (%d)\n", ret);
		perror("VIDIOC_S_FMT");
		return ret;
	}

	return ret;
}

int showCamSupportedFmt(int fd)
{
	//Check Supported Format
	struct v4l2_fmtdesc fmtdesc;
	memset(&fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("Supported format:\n");
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
	{
		printf("\t%d.%s\n", fmtdesc.index+1, fmtdesc.description);
		fmtdesc.index++;
	}

	return 0;
}

int showCamFrameSetting(int fd)
{
	//Show Current Frame Setting
	printf("%s\n", __func__);

	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret = ioctl(fd, VIDIOC_G_FMT, &fmt);
	if(ret < 0)
	{
		perror("VIDIOC_G_FMT");
		return -1;
	}
	printf("Current data format information:\n");
	//dumpCamFmt(&fmt);

	struct v4l2_fmtdesc fmtdesc;	
	memset(&fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
	{
		if(fmtdesc.pixelformat & fmt.fmt.pix.pixelformat)
		{
			printf("\tformat:%s\n", fmtdesc.description);
			break;
		}

		fmtdesc.index++;
	}


	return 0;
}

int isSupportRGB32(int fd)
{
	//Check if support a format
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
	if(ioctl(fd, VIDIOC_TRY_FMT, &fmt) == -1)
	{
		if(errno == EINVAL)
		{
			printf("Do not support format RGB32!\n");
			return -1;
		}
	}

	return 0;
}

int dumpV4l2Buf(struct v4l2_buffer* pBuf)
{
	printf("BufInfo:\n");
	printf("\tindex:%u\n", pBuf->index);
	printf("\ttype:%u\n", pBuf->type);
	printf("\tbytesused:%u\n", pBuf->bytesused);
	printf("\tflags:%u\n", pBuf->flags);
	
	//flags
	if(pBuf->flags & V4L2_BUF_FLAG_MAPPED)
	{
		printf("\t\tV4L2_BUF_FLAG_MAPPED\n");
	}

	if(pBuf->flags & V4L2_BUF_FLAG_QUEUED)
	{
		printf("\t\tV4L2_BUF_FLAG_QUEUED\n");
	}

	if(pBuf->flags & V4L2_BUF_FLAG_DONE)
	{
		printf("\t\tV4L2_BUF_FLAG_DONE\n");
	}
	
	if(pBuf->flags & V4L2_BUF_FLAG_KEYFRAME)
	{
		printf("\t\tV4L2_BUF_FLAG_KEYFRAME\n");
	}

	if(pBuf->flags & V4L2_BUF_FLAG_PFRAME)
	{
		printf("\t\tV4L2_BUF_FLAG_PFRAME\n");
	}

	if(pBuf->flags & V4L2_BUF_FLAG_BFRAME)
	{
		printf("\t\tV4L2_BUF_FLAG_BFRAME\n");
	}

	if(pBuf->flags & V4L2_BUF_FLAG_ERROR)
	{
		printf("\t\tV4L2_BUF_FLAG_ERROR\n");
	}
	
	printf("\ttimestamp:%u.%u\n", pBuf->timestamp.tv_sec, pBuf->timestamp.tv_usec);
	printf("\ttimecode:\n");
	printf("\t\ttype:%u\n", pBuf->timecode.type);
	printf("\t\tflags:%u\n", pBuf->timecode.flags);
	printf("\t\tframes:%u\n", pBuf->timecode.frames);
	printf("\t\t[%u:%u:%u]\n",
		pBuf->timecode.hours,
		pBuf->timecode.minutes,
		pBuf->timecode.seconds);
	printf("\tsequence:%u\n", pBuf->sequence);
}


//Show the current input video supported format.
int showInputSupport(int fd)
{
	struct v4l2_input input;
	memset(&input, 0, sizeof(struct v4l2_input));
	//Get the Current input index.
	if(-1 == ioctl(fd, VIDIOC_G_INPUT, &input.index))
	{
		perror("VIOIOC_G_INPUT");
		return -1;
	}
	
	//Get the Information matched to the index
	if(-1 == ioctl(fd, VIDIOC_ENUMINPUT, &input))
	{
		perror("VIDIOC_ENUMINPUT");
		return -1;
	}

	printf("Current input %s supports:\n", input.name);
	printf("\tindex:%d\n", input.index);
	printf("\tname:%s\n", input.name);
	printf("\ttype:%d\n", input.type);
	printf("\taudioset:%d\n", input.audioset);
	printf("\ttuner:%d\n", input.tuner);
	printf("\tstatus:%d\n", input.status);

	return 0;
}

//This function is not working.
int showCamStandard(int fd)
{
	struct v4l2_input input;
	memset(&input, 0, sizeof(struct v4l2_input));
	//Get the Current input index.
	if(-1 == ioctl(fd, VIDIOC_G_INPUT, &input.index))
	{
		perror("VIOIOC_G_INPUT");
		return -1;
	}
	
	//Get the Information matched to the index
	if(-1 == ioctl(fd, VIDIOC_ENUMINPUT, &input))
	{
		perror("VIDIOC_ENUMINPUT");
		return -1;
	}

	struct v4l2_standard standard;
	memset(&standard, 0, sizeof(struct v4l2_standard));
	standard.index = 0;

	//Get all the standards that the camera support.
	while(0 == ioctl(fd, VIDIOC_ENUMSTD, &standard))
	{
		printf("standard.index:%d\n", standard.index);

		if(standard.id & input.std)
		{
			printf("%s\n", standard.name);
		}

		standard.index++;
	}

	//EINVAL indicates the end of the enumeration.
	if(errno != EINVAL || standard.index == 0)
	{
		perror("VIDIOC_ENUMSTD");
		return -1;
	}

	//
	v4l2_std_id std_id;
	memset(&standard, 0, sizeof(standard));
	if(-1 == ioctl(fd, VIDIOC_G_STD, &std_id))
	{
		perror("VIDIOC_G_STD");
		return -1;
	}

	memset(&standard, 0, sizeof(standard));
	standard.index = 0;
	while(0 == ioctl(fd, VIDIOC_ENUMSTD, &standard))
	{
		if(standard.id & std_id)
		{
			printf("Current video standard:%s\n", standard.name);
			break;
		}
		standard.index++;
	}

	if(errno == EINVAL || standard.index == 0)
	{
		perror("VIDIOC_ENUMSTD");
		return -1;
	}

	return 0;
}

int mapAndEnqueueBuf(int fd, struct v4l2_requestbuffers *pReq, struct buffer* pFrameBuf)
{
	//Map the Frame Cache Buffer to User Application Buffer.
	int i;
	for(i = 0; i < pReq->count; i++)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		
		//Get phy addr and size.
		if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
		{
			perror("VIDIOC_QUERYBUF");
			return -1;
		}

		//Map buffer
		pFrameBuf[i].length = buf.length;
		pFrameBuf[i].start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if(MAP_FAILED == pFrameBuf[i].start)
		{
			perror("MMAP error");
			return -1;
		}

		//Queue buffer
		ioctl(fd, VIDIOC_QBUF, &buf);
	}

	return 0;
}

int main()
{
	char devStr[64] = {0};
	sprintf(devStr, "/dev/video%d", 1);
	int camFd = openCam(devStr);
	if(camFd < 0)
	{
		printf("Open camera device %s error\n", devStr);
		return -1;
	}	

	getCamInfo(camFd);
	showCamSupportedFmt(camFd);

	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	loadCamFmt(&fmt);
	setCamFmt(camFd, &fmt);
	getCamFmt(camFd);
	showCamFrameSetting(camFd);
	showInputSupport(camFd);
	
	//Request Frame Buffers
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));
	req.count = BUFFER_COUNT;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if(0 > ioctl(camFd, VIDIOC_REQBUFS, &req))
	{
		perror("VIDIOC_REQBUFS");
		return -1;
	}

	//Allocate Memory for buffers.
	struct buffer* buffers = NULL;
	buffers = (struct buffer*)calloc(BUFFER_COUNT, sizeof(*buffers));
	if(!buffers)
	{
		fprintf(stderr, "Out of memory\n");
		return -1;
	}

	mapAndEnqueueBuf(camFd, &req, buffers);
	
	//Start the Video Capture.
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(camFd, VIDIOC_STREAMON, &type);

	//
	while(1)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ioctl(camFd, VIDIOC_DQBUF, &buf);
		//dumpV4l2Buf(&buf);
		process_frame(buffers[buf.index].start, buf.bytesused);
		ioctl(camFd, VIDIOC_QBUF, &buf);	
	}

	if(camFd)
	{
		close(camFd);
	}			
	return 0;
}



