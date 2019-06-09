FROM tiangolo/uwsgi-nginx-flask:python3.7

# Install LibreOffice + deps + prepare Flask app to be ran
RUN pip3 install flask_login flask_sqlalchemy \
  && wget https://downloadarchive.documentfoundation.org/libreoffice/old/6.0.0.3/deb/x86_64/LibreOffice_6.0.0.3_Linux_x86-64_deb.tar.gz \
  && tar xvzf LibreOffice_6.0.0.3_Linux_x86-64_deb.tar.gz \
  && dpkg -i LibreOffice_6.0.0.3_Linux_x86-64_deb/DEBS/*.deb \
  && set -xe \
  && apt-get update \
  && apt-get install -y libdbus-1-dev libdbus-1-3 libdbus-glib-1-2 \
  && mkdir /tmp/ctf \
  && useradd -ms /home/libreoffice_admin -s /bin/bash libreoffice_admin

RUN echo "fb{wh0_7h0u6h7_l1br30ff1c3_c4n_b3_u53ful}" > /home/libreoffice_admin/flag

COPY ./app /app
WORKDIR /app

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/start.sh"]
