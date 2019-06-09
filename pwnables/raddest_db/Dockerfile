FROM ubuntu:18.04

ENV USER raddest_db

# setup user
RUN useradd $USER

# setup xinetd
RUN apt-get update
RUN apt-get install -y xinetd
COPY $USER.xinetd /etc/xinetd.d/$USER

# setup home files
COPY $USER /home/$USER/$USER
COPY flag /home/$USER/flag

# making the owner root prevents users from using chmod on the directory and its files
RUN chown -R root:$USER /home/$USER

# root and $USER can access and list files in the directory, but cannot create or delete files
RUN chmod -R 550 /home/$USER

# not very important, but remove the execute bit set on the flag because of the command above
RUN chmod -x /home/$USER/flag

EXPOSE 2301
CMD service xinetd start && sleep 2 && tail -f /var/log/xinetdlog
