#include <stdio.h>
#include <stdlib.h>

#define WIDTH 1024
#define HEIGHT 768
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct buffer {
  void   *start;
  size_t length;
};

int open_device(char *device);
int get_frame(FILE *frame_file,
              int device_fd,
              unsigned int *actual_width,
              unsigned int *actual_height);
