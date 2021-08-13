
FROM debian:buster-slim as builder

# Install packages for building
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install --no-install-recommends -y \
  build-essential \
  curl wget git ca-certificates \
  libpcre3 libpcre3-dev libssl-dev zlib1g-dev

# set workspace
WORKDIR /workspace

RUN git clone https://github.com/sergey-dryabzhinsky/nginx-rtmp-module.git
RUN mkdir -p nginx && \
  wget --progress=dot:mega http://nginx.org/download/nginx-1.21.1.tar.gz && \
  tar -zxf nginx-1.21.1.tar.gz && \
  cd nginx-1.21.1 && \
  ./configure --prefix=../nginx --with-http_ssl_module --add-module=../nginx-rtmp-module && \
  make -j 2 && \
  make install && \
  cp ../nginx-rtmp-module/stat.xsl ../nginx/html/


FROM debian:buster-slim 

# Install common tools
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install --no-install-recommends -y \
  curl wget git vim zip unzip tree stow procps net-tools iputils-ping dnsutils \
  lsb-release locales-all ca-certificates \
  && rm -rf /var/lib/apt/lists/*

COPY --from=builder /workspace/nginx /usr/local/nginx/
RUN cd /usr/local/nginx && stow -v .

COPY nginx.conf /usr/local/nginx/conf/

# for shown on runtime
ARG IMAGE_TAG
ENV IMAGE_TAG ${IMAGE_TAG}
RUN echo IMAGE_TAG=${IMAGE_TAG} >> /etc/environment

# labels
LABEL maintainer="wangyoucao577@gmail.com"
LABEL org.opencontainers.image.source = "https://github.com/wangyoucao577/multimedia-experiments"

# expose nginx rtmp & http port
EXPOSE 1935 
EXPOSE 8080

STOPSIGNAL SIGQUIT

# forward nginx access and error logs to docker log collector
RUN ln -sf /dev/stdout /usr/local/nginx/logs/access.log && ln -sf /dev/stderr /usr/local/nginx/logs/error.log

WORKDIR /usr/local/nginx

ENTRYPOINT ["nginx", "-g", "daemon off;"]
