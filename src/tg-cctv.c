#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include "tg.h"
#include "cam.h"
#include "log.h"

#include "lodepng.h"

#define IMAGE_BUFFER_SIZE WIDTH*HEIGHT*3

uint8_t RAW_IMAGE_BUFFER[IMAGE_BUFFER_SIZE] = {0};

int main(){
  log_set_level(LOG_INFO);
  log_info("Started");
  int device_fd = open_device("/dev/video0");
  if (device_fd < 0){
    log_info("Device open error, exiting");
    return 1;
  }

  unsigned int actual_width;
  unsigned int actual_height;

  FILE *raw_img_file = fmemopen(RAW_IMAGE_BUFFER, sizeof(RAW_IMAGE_BUFFER), "wb");
  get_frame(raw_img_file, device_fd, &actual_width, &actual_height);

  size_t png_size = 0;
  uint8_t *png_img_buf = NULL;
  unsigned int err = lodepng_encode24(&png_img_buf,
                                      &png_size,
                                      RAW_IMAGE_BUFFER,
                                      actual_width,
                                      actual_height);
  if (err){
    log_error(lodepng_error_text(err));
  }

  FILE *png_img_file = fopen("image.png", "wb");
  if (!png_img_file){
    goto error;
  }
  fwrite(png_img_buf, 1, png_size, png_img_file);
  fclose(png_img_file);

  free(png_img_buf);
  return 0;
error:
  if (png_img_buf){
    free(png_img_buf);
  }
  return 1;
}
