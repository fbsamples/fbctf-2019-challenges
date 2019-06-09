FROM tiangolo/uwsgi-nginx-flask:python3.7

RUN set -e -x ;\
    pip3 install flask_login; \
    pip3 install flask_sqlalchemy

COPY ./app /app
WORKDIR /app

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/start.sh"]
