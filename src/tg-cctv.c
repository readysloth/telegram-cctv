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

int main(int argc, char *argv[]){
  int ret = 0;
  int device_fd = -1;
  unsigned int actual_width = 0;
  unsigned int actual_height = 0;
  FILE *raw_img_file = NULL;

  size_t png_size = 0;
  uint8_t *png_img_buf = NULL;

  char *video_device = argc < 2 ? "/dev/video0" : argv[1];

  log_set_level(LOG_INFO);
  log_info("Started");
  device_fd = open_device(video_device);
  if (device_fd < 0){
    log_info("Device open error, exiting");
    goto error;
  }

  raw_img_file = fmemopen(RAW_IMAGE_BUFFER, sizeof(RAW_IMAGE_BUFFER), "wb");
  if (!raw_img_file){
    goto error;
  }

  get_frame(raw_img_file, device_fd, &actual_width, &actual_height);

  unsigned int err = lodepng_encode24(&png_img_buf,
                                      &png_size,
                                      RAW_IMAGE_BUFFER,
                                      actual_width,
                                      actual_height);
  if (err){
    log_error(lodepng_error_text(err));
  }

  upload_file(png_img_buf, png_size, TG_BOT_SEND_PHOTO_ENDPOINT);

  goto cleanup;

error:
  ret = 1;
cleanup:
  if (device_fd > 0){
    close_device(device_fd);
  }
  if (!raw_img_file){
    fclose(raw_img_file);
  }
  if (png_img_buf){
    free(png_img_buf);
  }
  return ret;
}
