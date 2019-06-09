FROM php:7.1.20-apache

COPY src/*.php /var/www/html/
COPY config/php/php.ini /usr/local/etc/php/php.ini
COPY config/vhosts /etc/apache2/sites-enabled

RUN a2enmod rewrite
RUN service apache2 restart
RUN docker-php-ext-install pdo pdo_mysql mysqli