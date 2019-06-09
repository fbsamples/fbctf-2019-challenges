FROM php:7.1.20-apache

RUN apt-get -y update --fix-missing \
  && apt-get upgrade -y \
  && apt-get -y install apt-utils nano wget dialog \
  && apt-get -y install --fix-missing apt-utils build-essential git curl libcurl3 libcurl3-dev zip \
  && curl -sS https://getcomposer.org/installer | php -- --install-dir=/usr/local/bin --filename=composer \
  && pecl install xdebug-2.5.0 && docker-php-ext-enable xdebug \
  && apt-get -y install libmcrypt-dev \
  && docker-php-ext-install mcrypt \
  && apt-get install -y libpq-dev \
  && docker-php-ext-configure pgsql -with-pgsql=/usr/local/pgsql \
  && docker-php-ext-install pdo pdo_pgsql pgsql \
  && docker-php-ext-install curl

RUN docker-php-ext-install tokenizer \
  && docker-php-ext-install json \
  && apt-get -y install zlib1g-dev \
  && docker-php-ext-install zip \
  && apt-get -y install libicu-dev \
  && docker-php-ext-install -j$(nproc) intl \
  && docker-php-ext-install mbstring \
  && apt-get install -y libfreetype6-dev libjpeg62-turbo-dev libpng-dev \
  && docker-php-ext-configure gd --with-freetype-dir=/usr/include/ --with-jpeg-dir=/usr/include/ \
  && docker-php-ext-install -j$(nproc) gd \
  && a2enmod rewrite headers

COPY www /var/www/html
COPY config/php/php.ini /usr/local/etc/php/php.ini
COPY config/vhosts /etc/apache2/sites-enabled
