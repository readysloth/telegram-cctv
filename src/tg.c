#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <curl/curl.h>

#include "tg.h"
#include "log.h"


CURLcode upload_buffer(uint8_t *buffer, size_t size, char *url){
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;
  curl_mime *mime = NULL;
  curl_mimepart *part = NULL;

  curl = curl_easy_init();
  if(!curl) {
    return -1;
  }

  mime = curl_mime_init(curl);
  part = curl_mime_addpart(mime);

  curl_mime_name(part, "photo");
  curl_mime_type(part, "image/jpeg");
  curl_mime_filename(part, "image.jpg");
  curl_mime_data(part, buffer, size);

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  res = curl_easy_perform(curl);
  if(res != CURLE_OK) {
    log_error("curl_easy_perform() failed with [0x%X:%s] on url %s\n",
              res,
              curl_easy_strerror(res),
              url);
  }
  curl_easy_cleanup(curl);
  curl_mime_free(mime);
  return res;
}
