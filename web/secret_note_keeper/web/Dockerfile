FROM tiangolo/uwsgi-nginx-flask:python3.7

#install deps
RUN set -e -x ;\
        apt-get update; \
        apt-get -y install mysql-server; \
        apt-get -y install libmariadbclient-dev; \
        apt-get -y install wget

#install pips
RUN set -e -x ;\
		pip3 install mysqlclient; \
		pip3 install flask; \
    pip3 install flask_login; \
    pip3 install flask_sqlalchemy;

COPY ./app /app
WORKDIR /app

RUN set -e -x ;\
  sed -i 's/127.0.0.1/0.0.0.0/' /etc/mysql/mariadb.conf.d/50-server.cnf; \
	service mysql start; \
	mysql -u root -e "use mysql; UPDATE user set authentication_string=PASSWORD('jkfhkkcftbutdgvkuckvkvrlbfrc00l') where user='root'; flush privileges; CREATE DATABASE IF NOT EXISTS notes; GRANT ALL PRIVILEGES ON notes.* to chal1@'%' IDENTIFIED BY 'getbvllnbunnhriunuglchillki';"; \
	mysql -u root notes < /app/notes_dump.sql;


EXPOSE 80


#RUN set -e -x ;\
#	groupadd -g 1337 user ;\
#	useradd -g 1337 -u 1337 -m user


#RUN set -e -x ;\
#	chown user:user /app ;\
#    chmod 500 /app

# USER user
ENTRYPOINT ["/entrypoint.sh"]
CMD service mysql start && /start.sh
