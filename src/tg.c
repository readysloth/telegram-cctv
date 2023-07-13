#include <stdio.h>
#include <stdbool.h>

#include <sys/stat.h>

#include <curl/curl.h>

#include "tg.h"
#include "log.h"


int upload_file(FILE *file, char *url){
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct stat file_info;

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, true);
    curl_easy_setopt(curl, CURLOPT_READDATA, file);

    curl_easy_setopt(curl,
                     CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      log_error("curl_easy_perform() failed with [%s] on url %s\n",
                url,
                curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
  }
  return res;
}
