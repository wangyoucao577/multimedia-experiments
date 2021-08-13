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

## Run container

```bash
$ docker run -d --restart=always -p 80:8080 -p 1935:1935 --shm-size=32g ghcr.io/wangyoucao577/multimedia-experiments/nginx
4dce06ca751f4ee803df809396b8e1ed830f496ed530f52476034a92d35c8fe2
$ 
```



## Author
wangyoucao577@gmail.com
