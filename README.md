# telegram-cctv

This small project implements capturing photos from video-device via v4l2 and sending
them to telegram channel.

It has minimum of dependencies: libcurl and libv4l2.

## Building

If dependencies are present, just run

```
make CFLAGS='-DTG_BOT_TOKEN="\"*TOKEN_FROM_BOTFATHER*\"" -DTG_CHANNEL="\"*CHANNEL_ID_OR_NAME*\""'
```
