#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

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

  size_t png_size = 0;
  struct device_info info = {0};
  char *video_device = argc < 2 ? "/dev/video0" : argv[1];
  bool disable_usb = argc < 3 ? false : !strcmp(argv[2], "disable_usb");;

  log_set_level(LOG_INFO);
  log_info("Started");
  log_info(disable_usb ? "ALL USB devices will be disabled in every iteration" : "As usual");

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

    int actual_size = get_frame(info.fd,
                                info.fmt,
                                RAW_IMAGE_BUFFER,
                                IMAGE_BUFFER_SIZE);
    upload_buffer(RAW_IMAGE_BUFFER, actual_size, TG_BOT_SEND_PHOTO_ENDPOINT);
    if (disable_usb){
      log_info("Disabling USB's");
      close_device(info.fd);
      info.fd = 0;
      info.fmt = NULL;
      ret = system("echo " USB_HUB " > /sys/bus/usb/drivers/usb/unbind");
      if (ret != 0) {
        log_error("%d, %s", errno, strerror(errno));
      }
    }
    sleep(3*60);
    if (disable_usb){
      log_info("Enabling USB's back");
      ret = system("echo " USB_HUB " > /sys/bus/usb/drivers/usb/bind");
      if (ret != 0) {
        log_error("%d, %s", errno, strerror(errno));
      }
      sleep(10);
    }
  }

  goto cleanup;

error:
  ret = 1;
cleanup:
  if (info.fd > 0){
    close_device(info.fd);
  }
  return ret;
}
