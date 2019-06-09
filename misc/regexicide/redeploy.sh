#!/usr/bin/env bash
rm -f /var/www/public/*
cp /var/www/index.php /var/www/public/index.php
chmod u+x /var/www/public/index.php
echo 'hey' >> /var/www/public/test
