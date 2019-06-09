FROM debian:stretch-slim

# Install deps + add Chrome Stable + purge all the things
RUN apt-get update && apt-get install -y \
	apt-transport-https \
	ca-certificates \
  mysql-client \
  python3 \
	python3-dev \
#	chromedriver \
	unzip \
	python3-pip \
	python3-setuptools \
	curl \
	gnupg \
	--no-install-recommends \
	&& curl -sSL https://dl.google.com/linux/linux_signing_key.pub | apt-key add - \
	&& echo "deb https://dl.google.com/linux/chrome/deb/ stable main" > /etc/apt/sources.list.d/google-chrome.list \
	&& apt-get update && apt-get install -y \
	google-chrome-stable \
	fontconfig \
	fonts-ipafont-gothic \
	fonts-wqy-zenhei \
	fonts-thai-tlwg \
	fonts-kacst \
	fonts-symbola \
	fonts-noto \
	ttf-freefont \
	--no-install-recommends \
	&& rm -rf /var/lib/apt/lists/* \
	&& pip3 install PyMySQL \
	&& pip3 install selenium==3.13.0


# Add Chrome as a user
RUN groupadd -r chrome && useradd -r -g chrome -G audio,video chrome \
    && mkdir -p /home/chrome && chown -R chrome:chrome /home/chrome \
		&& mkdir -p /opt/google/chrome && chown -R chrome:chrome /opt/google/chrome \
    && chown root:root /opt/google/chrome/chrome-sandbox \
    && chmod 4755 /opt/google/chrome/chrome-sandbox


RUN CHROMEDRIVER_VERSION=`curl -sS chromedriver.storage.googleapis.com/LATEST_RELEASE` && \
	  mkdir -p /opt/chromedriver-$CHROMEDRIVER_VERSION && \
		curl -sS -o /tmp/chromedriver_linux64.zip http://chromedriver.storage.googleapis.com/$CHROMEDRIVER_VERSION/chromedriver_linux64.zip && \
	  unzip -qq /tmp/chromedriver_linux64.zip -d /opt/chromedriver-$CHROMEDRIVER_VERSION && \
	  rm /tmp/chromedriver_linux64.zip && \
	  chmod +x /opt/chromedriver-$CHROMEDRIVER_VERSION/chromedriver && \
	  ln -fs /opt/chromedriver-$CHROMEDRIVER_VERSION/chromedriver /usr/local/bin/chromedriver \
		&& apt-get purge --auto-remove -y curl gnupg unzip


COPY ./bot /bot
WORKDIR /bot

# Run Chrome non-privileged
USER chrome


# Autorun chrome headless with no GPU
#CMD [ "--headless", "--disable-gpu", "--remote-debugging-address=0.0.0.0", "--remote-debugging-port=9222" ]
ENTRYPOINT python3 /bot/admin_bot.py
