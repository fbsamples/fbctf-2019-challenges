FROM ubuntu

RUN mkdir /thrift-challenge
WORKDIR /thrift-challenge/
COPY thrift-challenge /thrift-challenge/
COPY docker-entrypoint.sh /thrift-challenge/

RUN chmod +x docker-entrypoint.sh

RUN useradd -ms /bin/bash thrift
USER thrift

ENTRYPOINT ["./docker-entrypoint.sh"]
