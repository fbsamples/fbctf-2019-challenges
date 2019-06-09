FROM ubuntu:18.04

ENV USER postquantumsig

# setup user
RUN useradd $USER

# setup xinetd and python
RUN apt-get update \
  && apt-get install -y xinetd \
  && apt-get install -y python3
COPY $USER.xinetd /etc/xinetd.d/$USER

# setup home files
COPY src/solution_verifier.py /home/$USER/solution_verifier.py
COPY dist/signatures.csv /home/$USER/signatures.csv

# making the owner root prevents users from using chmod on the directory and its files
RUN chown -R root:$USER /home/$USER

# root and $USER can access and list files in the directory, but cannot create or delete files
RUN chmod -R 550 /home/$USER

EXPOSE 8080
CMD service xinetd start && sleep 2 && tail -f /var/log/xinetdlog
