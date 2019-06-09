FROM ubuntu:18.04

ENV USER frank

# setup user
RUN useradd $USER

# setup python
RUN apt-get update
RUN apt-get install -y python3 python3-pip
RUN pip3 install cryptography

# setup home files
COPY src /home/$USER/$USER
COPY flag /home/$USER/flag

# making the owner root prevents users from using chmod on the directory and its files
RUN chown -R root:$USER /home/$USER

# root and $USER can access and list files in the directory, but cannot create or delete files
RUN chmod -R 550 /home/$USER

# not very important, but remove the execute bit set on the flag because of the command above
RUN chmod -x /home/$USER/flag

# make the other python files not executable, then make the server executable again
RUN chmod -x /home/$USER/$USER/*.py
RUN chmod +x /home/$USER/$USER/handler.py

EXPOSE 4567
WORKDIR /home/$USER
CMD ["frank/handler.py"]
