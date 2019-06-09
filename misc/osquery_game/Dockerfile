FROM ubuntu

RUN apt-get update \
  && apt-get install -y openssh-server \
  && mkdir /var/run/sshd \
  && echo "X11Forwarding no" >> /etc/ssh/sshd_config \
  && echo "AllowTcpForwarding no" >> /etc/ssh/sshd_config \
  && sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile \
  && mkdir -p /usr/lib/osquery/extensions

COPY osqueryi /bin/osquerygame
COPY external_extension_fbctf2019.ext /usr/lib/osquery/extensions/

RUN echo "ClientAliveInterval 60" >> /etc/ssh/sshd_config \
  && echo "ClientAliveCountMax 0" >> /etc/ssh/sshd_config

RUN mkdir -p /etc/osquery \
  && echo "/usr/lib/osquery/extensions/external_extension_fbctf2019.ext" > /etc/osquery/extensions.load \
  && mkdir /home/osquerygame \
  && useradd -s /bin/osquerygame -d /home/osquerygame -p '$1$GmIBmaD0$vSJ5n3rmmtjP3Nkl5Oxdn1' osquerygame \
  && chown osquerygame /home/osquerygame \
  && echo "KEY=\"fb{you_win_the_game_again_again}\"" >> /etc/environment \
  && touch /home/osquerygame/.hushlogin \
  && echo "osquerygame - nofile 1048576" >> /etc/security/limits.conf \
  && mkdir -p /home/osquerygame/.osquery \
  && chown osquerygame /home/osquerygame/.osquery \
  && touch /home/osquerygame/.osquery/.history \
  && echo ".schema" > /home/osquerygame/.osquery/.history \
  && echo "SELECT * FROM farm_quests;" >> /home/osquerygame/.osquery/.history \
  && echo "SELECT * FROM farm_actions;" >> /home/osquerygame/.osquery/.history \
  && echo "SELECT * FROM farm;" >> /home/osquerygame/.osquery/.history \
  && echo "SELECT to_base64('testing');" >> /home/osquerygame/.osquery/.history \
  && echo "SELECT * FROM farm WHERE action = 'move' AND src = 0xAD AND dst = 0xAF;" >> /home/osquerygame/.osquery/.history \
  && echo "SELECT * FROM farm WHERE action = 'pickup' AND src = 0xAF;" >> /home/osquerygame/.osquery/.history \
  && echo "--disable_watchdog" > /etc/osquery/osquery.flags.default

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
