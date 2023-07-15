#include <stdio.h>

#include <curl/curl.h>

#ifndef TG_CHANNEL
  #error You should define TG_CHANNEL
#endif

#ifndef TG_BOT_TOKEN
  #error You should define TG_BOT_TOKEN
#endif

#define TG_BASE_API_URL "https://api.telegram.org"
#define TG_BOT_ENDPOINT TG_BASE_API_URL "/bot" TG_BOT_TOKEN
#define TG_BOT_SEND_PHOTO_ENDPOINT TG_BOT_ENDPOINT "/sendPhoto?chat_id=" TG_CHANNEL


CURLcode upload_file(uint8_t *buffer, size_t size, char *url);
