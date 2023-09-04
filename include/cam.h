#include <stdio.h>
#include <stdlib.h>

#define WIDTH 1024*2
#define HEIGHT 768*2
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct buffer {
  void   *start;
  size_t length;
};

int open_device(char *device);
int close_device(int fd);
void *device_setup(int device_setup,
                   unsigned int *actual_width,
                   unsigned int *actual_height);
int get_frame(int device_fd,
              void *fmt_ptr,
              uint8_t *output_buf,
              size_t output_buf_size);
