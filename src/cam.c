#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#include "log.h"
#include "cam.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define BUFFERS_COUNT 1
#define DEFAULT_V4L2_FMT {.type = V4L2_BUF_TYPE_VIDEO_CAPTURE, \
                          .fmt = { .pix = {.width = WIDTH, \
                                           .height = HEIGHT, \
                                           .pixelformat = V4L2_PIX_FMT_MJPEG,\
                                           .field = V4L2_FIELD_INTERLACED_TB}}}


static int xioctl(int fh, int request, void *arg){
  int r = 0;

  do {
    r = v4l2_ioctl(fh, request, arg);
  } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

  if (r == -1) {
    log_error("%d, %s", errno, strerror(errno));
  }
  return r;
}


int open_device(char *device){
  static int fd = -1;

  if (fd != -1) {
    return fd;
  }

  fd = v4l2_open(device, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    log_error("Cannot open device %s", device);
  }
  return fd;
}


int close_device(int fd){
  return v4l2_close(fd);
}


static int get_frame_from_device(int device_fd,
                                 struct buffer *buffers,
                                 struct v4l2_buffer buf,
                                 struct v4l2_format fmt,
                                 uint8_t *output_buffer,
                                 size_t output_buffer_size){
  fd_set fds;
  struct timeval tv;

  int ret = 0;

  do {
    FD_ZERO(&fds);
    FD_SET(device_fd, &fds);

    /* Timeout. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    ret = select(device_fd + 1, &fds, NULL, NULL, &tv);
  } while ((ret == -1 && (errno = EINTR)));
  if (ret == -1) {
    log_error("%d, %s", errno, strerror(errno));
    return -1;
  }

  ret = xioctl(device_fd, VIDIOC_DQBUF, &buf);
  log_debug("xioctl(device_fd, VIDIOC_DQBUF, &buf) = %i", ret);
  int size = buf.bytesused;
  memcpy(output_buffer,
         buffers[buf.index].start,
         MIN(buf.bytesused, output_buffer_size));
  ret = xioctl(device_fd, VIDIOC_QBUF, &buf);
  log_debug("xioctl(device_fd, VIDIOC_QBUF, &buf) = %i", ret);
  return size;
}


void *device_setup(int device_fd,
                   unsigned int *actual_width,
                   unsigned int *actual_height){
  static struct v4l2_format fmt = DEFAULT_V4L2_FMT;
  int ret = 0;

  ret = xioctl(device_fd, VIDIOC_S_FMT, &fmt);
  log_debug("xioctl(device_fd, VIDIOC_S_FMT, &fmt) = %i", ret);
  if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_MJPEG){
    log_error("Libv4l didn't accept MJPEG format. Can't proceed.");
    return NULL;
  }

  *actual_width = fmt.fmt.pix.width;
  *actual_height = fmt.fmt.pix.height;

  if ((fmt.fmt.pix.width != WIDTH) ||
      (fmt.fmt.pix.height != HEIGHT)){
    log_warn("Warning: driver is sending image at %dx%d, not at %dx%d",
             fmt.fmt.pix.width,
             fmt.fmt.pix.height,
             WIDTH,
             HEIGHT);
  }

  return &fmt;
}


int get_frame(int device_fd,
              void *fmt_ptr,
              uint8_t *output_buffer,
              size_t output_buffer_size){
  struct buffer buffers[BUFFERS_COUNT] = {0};
  struct v4l2_requestbuffers req = {.count = ARRAY_SIZE(buffers),
                                    .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
                                    .memory = V4L2_MEMORY_MMAP};
  struct v4l2_buffer buf = {.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
                            .memory = V4L2_MEMORY_MMAP,
                            .index = 0};
  struct v4l2_format fmt = *((struct v4l2_format*)fmt_ptr);

  int ret = 0;

  ret = xioctl(device_fd, VIDIOC_REQBUFS, &req);
  log_debug("xioctl(device_fd, VIDIOC_REQBUFS, &req) = %i", ret);

  for (int i = 0; i < req.count; i++) {
    buf.index = i;

    ret = xioctl(device_fd, VIDIOC_QUERYBUF, &buf);
    log_debug("xioctl(device_fd, VIDIOC_QUERYBUF, &buf) = %i", ret);

    buffers[i].length = buf.length;
    buffers[i].start = v4l2_mmap(NULL,
                                 buf.length,
                                 PROT_READ,
                                 MAP_SHARED,
                                 device_fd,
                                 buf.m.offset);
    if (MAP_FAILED == buffers[i].start) {
      log_error("mmaping of video-device failed");
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < req.count; i++) {
    buf.index = i;
    ret = xioctl(device_fd, VIDIOC_QBUF, &buf);
    log_debug("xioctl(device_fd, VIDIOC_QBUF, &buf) = %i", ret);
  }

  xioctl(device_fd, VIDIOC_STREAMON, &req.type);
  log_debug("xioctl(device_fd, VIDIOC_STREAMON, &req.type) = %i", ret);

  int size = 0;
  // Allow cam to adjust the brightness
  for (int i = 0; i < 10; i++)
    size = get_frame_from_device(device_fd,
                                 buffers,
                                 buf,
                                 fmt,
                                 output_buffer,
                                 output_buffer_size);
  ret = xioctl(device_fd, VIDIOC_STREAMOFF, &req.type);
  log_debug("xioctl(device_fd, VIDIOC_STREAMOFF, &type) = %i", ret);
  for (int i = 0; i < req.count; i++) {
    v4l2_munmap(buffers[i].start, buffers[i].length);
  }
  return size;
}
