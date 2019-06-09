FROM python:2.7-alpine
RUN apk update && \
    apk add python python-dev linux-headers libffi-dev gcc make musl-dev py-pip mysql-client git openssl-dev

WORKDIR /opt/whysapp
RUN mkdir -p /opt/whysapp

COPY . /opt/whysapp

RUN pip install -r requirements.txt
RUN chmod +x /opt/whysapp/docker-entrypoint.sh

EXPOSE 5242
ENTRYPOINT ["/opt/whysapp/docker-entrypoint.sh"]
