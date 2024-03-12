#############################################
# debian
#############################################
FROM debian:11-slim

#############################################
# basics
#############################################
RUN apt-get update && apt-get install -y \
        rclone \
        git \
        make \
        jq \
        wget \
        curl

#############################################
# node
#############################################
RUN curl -sL https://deb.nodesource.com/setup_14.x | bash && \
    apt-get update && apt-get install -y nodejs && rm -rf /var/lib/apt/lists/*

RUN npm install -g gh-release

