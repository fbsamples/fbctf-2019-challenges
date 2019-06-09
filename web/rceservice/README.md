rceservice
==========

Challenge text: "We created this web interface to run commands on our servers, but since we haven't figured out how to secure it yet we only let you run 'ls'"

This problem is related to how PHP handles errors in `preg_match`. PHP has a configuration option for regex called `pcre.backtrack_limit`. If the regex engine exceeds this limit when matching, `preg_match` will return `false` indicating an error occurred. `preg_match` also returns `0` if no match is found, and this is equivalent to `false` in an `if`-statment. So we need to force this error by making the regex engine exceed the backtrack limit.

We can do this by adding a large number (i.e. 1,000,000) spaces after our JSON input. This causes any JSON to fail the blacklist regex, allowing our commands to be executed. `solve.py` makes a POST request to the server and sends `{"cmd": "/bin/cat /home/rceservice/flag"} + ' '*1000000`, which will make the server output the flag.

We jail this problem by creating a jail directory that only contains `ls` and setting the PATH to only that directory. The regex disallows all characters, except the characters that match the following regex (i.e. the whitelist corresponding to the blacklist): [a-z .{":}]. However, during the CTF there was still an unintended solution using newlines.

I would distribute the index.php source with this problem.

References
----------

- https://www.php.net/manual/en/pcre.configuration.php
- There's a comment from 9 years ago that backtrack limit could have security implications: https://www.php.net/manual/en/function.preg-match.php
