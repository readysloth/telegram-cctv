#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include "tg.h"
#include "cam.h"
#include "log.h"

#define IMAGE_BUFFER_SIZE WIDTH*HEIGHT*3

#define USB_HUB_DEFAULT "1-1"
#define USB_HUB "1-1"

#define WARNING_MSG "Default USB hub is " USB_HUB_DEFAULT \
                    " and specified USB hub is " USB_HUB "." \
                    " Perhaps you want to redefine it, if default matches specified."

#pragma message WARNING_MSG
#warning See the message above

uint8_t RAW_IMAGE_BUFFER[IMAGE_BUFFER_SIZE] = {0};

struct device_info {
  int fd;
  void *fmt;
};


struct device_info prepare_video_device(char *video_device,
                                        unsigned int *actual_width,
                                        unsigned int *actual_height){
  struct device_info info = {0};
  info.fd = open_device(video_device);
  if (info.fd < 0){
    log_info("Device open error");
    goto error;
  }

  info.fmt = device_setup(info.fd, actual_width, actual_height);
  if (!info.fmt){
    goto error;
  }

  return info;
error:
  if (info.fd > 0){
    close_device(info.fd);
  }
  info.fd = -1;
  info.fmt = NULL;
  return info;
}


int main(int argc, char *argv[]){
  int ret = 0;
  unsigned int actual_width = 0;
  unsigned int actual_height = 0;
  FILE *raw_img_file = NULL;

  size_t png_size = 0;
  struct device_info info = {0};
  char *video_device = argc < 2 ? "/dev/video0" : argv[1];
  bool disable_usb = argc < 3 ? false : !strcmp(argv[2], "disable_usb");;

  log_set_level(LOG_INFO);
  log_info("Started");
  log_info(disable_usb ? "ALL USB devices will be disabled in every iteration" : "As usual");

  raw_img_file = fmemopen(RAW_IMAGE_BUFFER, sizeof(RAW_IMAGE_BUFFER), "wb");
  if (!raw_img_file){
    goto error;
  }

  while(true){
    if (info.fd == 0){
      log_info("Preparing video device");
      info = prepare_video_device(video_device,
                                  &actual_width,
                                  &actual_height);
      if (info.fd == -1){
        log_error("Cannot prepare video device");
        continue;
      }
    }

    int size = get_frame(raw_img_file, info.fd, info.fmt);
    upload_buffer(RAW_IMAGE_BUFFER, size, TG_BOT_SEND_PHOTO_ENDPOINT);
    rewind(raw_img_file);
    if (disable_usb){
      log_info("Disabling USB's");
      close_device(info.fd);
      info.fd = 0;
      info.fmt = NULL;
      system("echo " USB_HUB " > /sys/bus/usb/drivers/usb/unbind");
    }
    sleep(3*60);
    if (disable_usb){
      log_info("Enabling USB's back");
      system("echo " USB_HUB " > /sys/bus/usb/drivers/usb/bind");
      sleep(5);
    }
  }

  goto cleanup;

error:
  ret = 1;
cleanup:
  if (info.fd > 0){
    close_device(info.fd);
  }
  if (raw_img_file){
    fclose(raw_img_file);
  }
  return ret;
}
