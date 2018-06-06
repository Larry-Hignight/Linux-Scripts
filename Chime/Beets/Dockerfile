#FROM alpine
FROM debian:unstable-slim

#RUN apk update && apk add mpv python3
RUN apt-get update && apt-get -y install python3 \
                                         ffmpeg \
					 libchromaprint-tools \
					 python3-pip

RUN pip3 install --upgrade pip && pip install beets pyacoustid

ADD config.yaml /root/.config/beets/config.yaml
WORKDIR /home
EXPOSE 5000