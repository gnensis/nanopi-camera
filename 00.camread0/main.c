#include "fimc.h"

static int fd_cam=-1 ;
static struct v4l2_requestbuffers reqbuf;
static VideoBuffer framebuf[BUFFER_COUNT];
static struct v4l2_buffer buf;

int hex_dump(char* buf, int buf_len)
{
	int i = 0;
	for(i = 0; i < buf_len; i++)
	{
		printf("%02x ", buf[i]);
		if((i + 1)%16 == 0)
		{
			printf("\n");
		}
	}

	printf("\n");

	return 0;
}

int Camera_OpenCam()
{
    // open device
    fd_cam = open(CAMERA_DEVICE, O_RDWR, 0);
    if (fd_cam < 0) {
        printf("Open %s failed\n", CAMERA_DEVICE);
        return -1;
    }
    return fd_cam ;
}
int Camera_CloseCam()
{
	return close(fd_cam);
}
int Camera_GetCamInfo()
{
	int ret=-1 ;
    // GetDevicInfo
    struct v4l2_capability cap;
    ret = ioctl(fd_cam, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
        return ret;
    }
    // Print capability infomations
    printf("Capability Informations:\n");
    printf(" driver: %s\n", cap.driver);
    printf(" card: %s\n", cap.card);
    printf(" bus_info: %s\n", cap.bus_info);
    printf(" version: %08X\n", cap.version);
    printf(" capabilities: %08X\n", cap.capabilities);

    return ret ;
}
int Camera_SetCamFMT()
{
	int ret = -1 ;
	//SetCamFMT
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = VIDEO_WIDTH;
	fmt.fmt.pix.height      = VIDEO_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	ret = ioctl(fd_cam, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		printf("VIDIOC_S_FMT failed (%d)\n", ret);
		return ret;
	}

	// GetCamFMT to Verify
	ret = ioctl(fd_cam, VIDIOC_G_FMT, &fmt);
	if (ret < 0) {
		printf("VIDIOC_G_FMT failed (%d)\n", ret);
		return ret;
	}
	// Print Stream Format
	printf("Stream Format Informations:\n");
	printf(" type: %d\n", fmt.type);
	printf(" width: %d\n", fmt.fmt.pix.width);
	printf(" height: %d\n", fmt.fmt.pix.height);
	char fmtstr[8];
	memset(fmtstr, 0, 8);
	memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
	printf(" pixelformat: %s\n", fmtstr);
	printf(" field: %d\n", fmt.fmt.pix.field);
	printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
	printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
	printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
	printf(" priv: %d\n", fmt.fmt.pix.priv);
	printf(" raw_date: %s\n", fmt.fmt.raw_data);

	return 0 ;
}
int Camera_RequestBuffers(struct v4l2_requestbuffers *reqbuf__)
{
    // Request Buffers from kernel
	int ret =-1 ;
	reqbuf__->count = BUFFER_COUNT;
	reqbuf__->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf__->memory = V4L2_MEMORY_MMAP;

    ret = ioctl(fd_cam , VIDIOC_REQBUFS, reqbuf__);
    if(ret < 0) {
        printf("VIDIOC_REQBUFS failed (%d)\n", ret);
        return ret;
    }
    return ret ;
}
int Camera_MapAndEnqueue(struct v4l2_requestbuffers *reqbuf__, struct v4l2_buffer *buf__,VideoBuffer *framebuf_)
{
	int i=-1,ret=-1 ;
	 // Map And Enqueue
	for (i = 0; i < reqbuf__->count; i++)
	{
		buf__->index = i;
		buf__->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf__->memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd_cam , VIDIOC_QUERYBUF, buf__);
		if(ret < 0) {
			printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
			return ret;
		}

		// mmap buffer
		framebuf_[i].length = buf__->length;
		framebuf_[i].start = (char *) mmap(0, buf__->length, PROT_READ|PROT_WRITE, MAP_SHARED, fd_cam, buf__->m.offset);
		if (framebuf_[i].start == MAP_FAILED) {
			printf("mmap (%d) failed: %s\n", i, strerror(errno));
			return -1;
		}

		// Queen buffer
		ret = ioctl(fd_cam , VIDIOC_QBUF, buf__);
		if (ret < 0) {
			printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
			return -1;
		}

		printf("Frame buffer %d: address=0x%x, length=%d\n", i, (unsigned int)framebuf_[i].start, framebuf_[i].length);
	}
	return 0 ;
}
void Camera_UNMap(VideoBuffer *framebuf_)
{
	int i=-1 ;
    // Release the resource
    for (i=0; i< 4; i++)
    {
        munmap(framebuf_[i].start, framebuf_[i].length);
    }
}
int Camera_StartCameraStreaming()
{
	int ret=-1 ;
    // start camera streaming
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd_cam, VIDIOC_STREAMON, &type);
    if (ret < 0) {
        printf("VIDIOC_STREAMON failed (%d)\n", ret);
        return ret;
    }
    return ret ;
}
int Camera_DeQueueBufferGetFrame( struct v4l2_buffer *buf_)
{
    int ret=-1 ;
    // De-queue buffer ,Get frame
    ret = ioctl(fd_cam, VIDIOC_DQBUF, buf_);
    if (ret < 0) {
        printf("VIDIOC_DQBUF failed (%d)\n", ret);
        return ret;
    }
    return ret ;
}
int Camera_EnQueueBuffer(struct v4l2_buffer *buf_)
{
	int ret = -1 ;
    // Re-queen buffer
    ret = ioctl(fd_cam, VIDIOC_QBUF, buf_);
    if (ret < 0) {
        printf("VIDIOC_QBUF failed (%d)\n", ret);
        return ret;
    }
    return 0 ;
}


void Camera_Record(const char* fileName ,struct v4l2_buffer *buf_)
{
    // record frame 
    FILE *fp = fopen(fileName, "a+");
    if (fp < 0) {
        printf("open frame data file failed\n");
        return ;
    }
    printf("bytesused:%ud\n", buf_->bytesused);
    //hex_dump(framebuf[buf_->index].start, 50);
    //hex_dump(framebuf[buf_->index].start + buf_->length - 20, 20);
    //hex_dump(framebuf[buf_->index].start + buf_->bytesused - 10, 20);
    fwrite(framebuf[buf_->index].start, 1, buf_->bytesused, fp);
    fclose(fp);
    printf("Capture one frame[%d bytes] saved in %s\n", buf_->bytesused, fileName);
}

void Camera_ClearBuffer(struct v4l2_buffer *buf)
{
    memset(framebuf[buf->index].start, 0, buf->length);
}

int main()
{
    Camera_OpenCam();
    Camera_GetCamInfo() ;
    Camera_SetCamFMT();

    Camera_RequestBuffers(&reqbuf);
    Camera_MapAndEnqueue(&reqbuf,&buf,framebuf);


    Camera_StartCameraStreaming();
    int i = 0;
    for(i = 0; i < 500; i++)
    {
    	Camera_DeQueueBufferGetFrame(&buf);
	printf("buf.index:%d\n", buf.index);
    	Camera_Record(CAPTURE_FILE,&buf);
	Camera_ClearBuffer(&buf);
    	Camera_EnQueueBuffer(&buf);
    }

    Camera_UNMap(framebuf);
    Camera_CloseCam();

    printf("Camera test Done.\n");
    return 0;
}

