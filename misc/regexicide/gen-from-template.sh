#!/usr/bin/env bash

PASSWORD=
FLAG=
REDEPLOY_FREQ=3

usage () {
  printf '%s [OPTIONS] [--]\n' "$0"
  printf '  Regenerates files for deployment\n'
  printf 'Options:\n'
  printf '  -h: Display this help and exit\n'
  printf '  -p: use given password (otherwise password is random)\n'
  printf '  -f: use given flag (otherwise flag is random)\n'
  printf '  -d: Redeploy every N seconds (default: %d)\n' "$REDEPLOY_FREQ"
}

function randomstring() {
  cat /dev/urandom | head -n 5 | md5sum | cut -d\  -f1
}

while getopts "p:f:d:h?" OPT; do
  case "$OPT" in
    p) PASSWORD="$OPTARG";;
    f) FLAG="$OPTARG";;
    d) REDEPLOY_FREQ="$OPTARG";;
    h|?) usage && exit 0;;
    *) usage && exit 1;;
  esac
done
shift $((OPTIND -1))
if [ $# -ne 0 ]; then
  usage && exit 1;
fi

if [ -z "$PASSWORD" ]; then
  PASSWORD="$(randomstring)"
fi
if [ -z "$FLAG" ]; then
  FLAG="$(randomstring)"
fi

printf 'Generated regexicide challenge with password %s flag %s\n' "$PASSWORD" "$FLAG"

sed -e 's/REPLACE_WITH_PASSWORD/$password="'"$PASSWORD"'";/' \
  -e 's/REPLACE_WITH_FLAG/$flag="'"$FLAG"'";/' < index.php.template > index.php

sed -e "s/REPLACE_WITH_SLEEP/sleep $REDEPLOY_FREQ/" < docker-entrypoint.sh.template > docker-entrypoint.sh
