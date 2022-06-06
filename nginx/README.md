# nginx
[![Build Docker - nginx](https://github.com/wangyoucao577/multimedia-experiments/actions/workflows/autobuild-nginx.yml/badge.svg)](https://github.com/wangyoucao577/multimedia-experiments/actions/workflows/autobuild-nginx.yml)     
`nginx` based docker image for living RTMP & HLS.     


## Build Image
- [Dockerfile](./Dockerfile)

```bash
$ cd nginx
$ docker build -t ghcr.io/wangyoucao577/multimedia-experiments/nginx .  
```

## Pull Image 
### DockerHub

### Github Container Registry
[Github Container Repo: wangyoucao577/multimedia-experiments/nginx](https://github.com/wangyoucao577/multimedia-experiments/pkgs/container/multimedia-experiments%2Fnginx)
```bash
$ docker pull ghcr.io/wangyoucao577/multimedia-experiments/nginx
```

## Run

### Run container 
```bash
# `--mount "src=$(pwd),dst=/files,type=bind"` could be removed if no need as a generic file server
$ docker run -d --restart=always -p 8080:8080 -p 1935:1935 --shm-size=32g --mount "src=$(pwd),dst=/files,type=bind" ghcr.io/wangyoucao577/multimedia-experiments/nginx
4dce06ca751f4ee803df809396b8e1ed830f496ed530f52476034a92d35c8fe2
$ 
```

### Stream RTMP then Play via HLS
Make sure you have installed latest `ffmpeg` already.        

```bash
$ # sample video file from https://download.blender.org/durian/trailer/
$ wget https://download.blender.org/durian/trailer/sintel_trailer-1080p.mp4 
$ 
$ # stream via RTMP
$ ffmpeg -re -stream_loop -1 -i sintel_trailer-1080p.mp4 -c:v copy -c:a copy -f flv rtmp://127.0.0.1:1935/live/test1
$
$ # in another terminal session, play via HLS
$ ffplay http://127.0.0.1:8080/hls/test1.m3u8
```


## Author
wangyoucao577@gmail.com
