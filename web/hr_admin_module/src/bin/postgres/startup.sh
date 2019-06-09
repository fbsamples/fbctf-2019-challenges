#!/bin/bash
set -e

printf 'fb{@@dns_3xfil_f0r_the_w1n!!@@}' > /var/lib/postgresql/data/secret

psql -v ON_ERROR_STOP=1 --username "postgres" <<-EOSQL
    CREATE USER docker WITH ENCRYPTED PASSWORD 'aYRr45lTgN9I9LJcjcr0';
    CREATE DATABASE docker_db;
EOSQL

psql -v ON_ERROR_STOP=1 --username "postgres" docker_db <<-EOSQL
    REVOKE ALL ON schema public FROM public;
    CREATE EXTENSION dblink;
    CREATE TABLE searches(id INT, search TEXT);
    GRANT CONNECT ON DATABASE docker_db TO docker;
    GRANT USAGE ON SCHEMA public TO docker;
    GRANT EXECUTE ON FUNCTION dblink_connect_u(text, text) TO docker;
    GRANT EXECUTE ON FUNCTION lo_import(text) TO docker;
    GRANT SELECT ON searches TO docker;
EOSQL
