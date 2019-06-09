FROM ubuntu:18.04

ENV USER storagespace

# setup user
RUN useradd $USER

# install python things
RUN apt-get update -y
RUN apt-get install -y python-pip libgmp10 libgmp-dev
RUN pip2 install sympy fastecdsa

# use precomputed curves
COPY src/curves.txt /srv/curves
RUN chown root:root /srv/curves
RUN chmod 444 /srv/curves

# setup home files
COPY src/challenge.py /home/$USER/challenge.py
COPY src/serve.sh /home/$USER/serve.sh
COPY flag /home/$USER/flag

# there should be no shells in this challenge, but just in case :)
# making the owner root prevents users from using chmod on the directory and its files
RUN chown -R root:$USER /home/$USER
RUN chmod -R 750 /home/$USER

# not very important, but remove the execute bit set on the flag because of the command above
RUN chmod -x /home/$USER/flag

EXPOSE 2302
CMD su $USER -c "cd && /home/$USER/serve.sh"
