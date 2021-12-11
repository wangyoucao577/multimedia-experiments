# ffmpeg-libav-tutorial - Learn ffmpeg libav the hard way
Many thanks to [leandromoreira](https://github.com/leandromoreira) who wrote the excellent tutorial [leandromoreira/ffmpeg-libav-tutorial#learn-ffmpeg-libav-the-hard-way](https://github.com/leandromoreira/ffmpeg-libav-tutorial/#learn-ffmpeg-libav-the-hard-way)!      

## Test environment    

```bash
# macOS Big Sur (Version 11.6.1)

$ clang --version
Apple clang version 13.0.0 (clang-1300.0.29.3)
Target: x86_64-apple-darwin20.6.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin

$ ffmpeg --version
ffmpeg version 4.4.1 Copyright (c) 2000-2021 the FFmpeg developers
  built with Apple clang version 13.0.0 (clang-1300.0.29.3)
  configuration: --prefix=/usr/local/Cellar/ffmpeg/4.4.1-with-options_2 --enable-shared --cc=clang --host-cflags= --host-ldflags= --enable-gpl --enable-libaom --enable-libdav1d --enable-libmp3lame --enable-libopus --enable-libsnappy --enable-libtheora --enable-libvmaf --enable-libvorbis --enable-libvpx --enable-libx264 --enable-libx265 --enable-libfontconfig --enable-libfreetype --enable-frei0r --enable-libass --enable-demuxer=dash --enable-opencl --enable-videotoolbox --disable-htmlpages --enable-libfdk-aac --enable-libopenh264 --enable-libsrt --enable-nonfree
  libavutil      56. 70.100 / 56. 70.100
  libavcodec     58.134.100 / 58.134.100
  libavformat    58. 76.100 / 58. 76.100
  libavdevice    58. 13.100 / 58. 13.100
  libavfilter     7.110.100 /  7.110.100
  libswscale      5.  9.100 /  5.  9.100
  libswresample   3.  9.100 /  3.  9.100
  libpostproc    55.  9.100 / 55.  9.100
```

## Hello world

```bash
$ clang $(pkg-config --cflags --libs libavcodec libavformat libavfilter libavdevice libswresample libswscale libavutil) src/0_hello_world.c -o build/hello
$ ./build/hello small_bunny_1080p_60fps.mp4
```

## Remuxing 

```bash
$ clang $(pkg-config --cflags --libs libavcodec libavformat libavfilter libavdevice libswresample libswscale libavutil) src/2_remuxing.c -o build/remuxing
$ ./build/remuxing small_bunny_1080p_60fps.mp4 remuxed_small_bunny_1080p_60fps.ts
```

