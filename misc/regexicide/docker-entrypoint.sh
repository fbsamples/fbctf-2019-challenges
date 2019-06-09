#!/usr/bin/env bash
function redeploy_loop() {
  while true; do
    rm -f /var/www/public/*
    cp /var/www/index.php /var/www/public/index.php
    chmod 500 /var/www/public/index.php
    printf 'Last deployed at %s\n' "$(date +%s)" > /var/www/public/deploy.log
    sleep 3
  done
}

redeploy_loop &
/usr/bin/hhvm -m server -c /etc/hhvm/server.ini -c /etc/hhvm/site.ini
