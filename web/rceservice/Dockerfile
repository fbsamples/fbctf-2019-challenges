FROM php:7.2-apache

ENV USER rceservice

# setup user
RUN useradd $USER

COPY src/index.php /var/www/html/index.php
COPY src/flag /home/$USER/flag
COPY ls /home/$USER/jail/ls

# making the owner root prevents users from using chmod on the directory and its files
RUN chown -R root:$USER /home/$USER

# root and $USER can access and list files in the directory, but cannot create or delete files
RUN chmod -R 555 /home/$USER

# not very important, but remove the execute bit set on the flag because of the command above
RUN chmod -x /home/$USER/flag

RUN chmod +x /home/$USER/jail/ls

EXPOSE 80
