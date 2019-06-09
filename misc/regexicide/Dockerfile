# TODO: this won't work when the vuln is fixed
FROM hhvm/hhvm-proxygen:latest

RUN apt-get update -y && apt-get install -y curl
# Install composer
RUN mkdir /opt/composer

# Install the app
RUN rm -rf /var/www
ADD . /var/www

# Reconfigure HHVM
ADD hhvm.prod.ini /etc/hhvm/site.ini
RUN chmod 500 /var/www/docker-entrypoint.sh
RUN mkdir /var/www/public
RUN chmod 777 /var/www/public
RUN chmod 400 /var/www/index.php

ENTRYPOINT ["/var/www/docker-entrypoint.sh"]

EXPOSE 80
