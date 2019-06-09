Secret Note keeper
==========
**Challenge description: (internal)**
This problem is related to leaking sensitive data cross origin using window.frames.length. A successful participant should report a bug to the admin with a link to visit, that link will exploit the CSRF in the search query to search for a note with `CTF{SOME_TEXT}`. Using the CSRF and as the search page return results in frames, the attacker can leak the content of the search result. The attacker can then leak the full text of the flag.

**Challenge description: (external)**
Find the secret note that contains the fl4g!


**Run with:**
```
docker-compose up
```

**Solution description:**
CSRF to the search page using window.frames
