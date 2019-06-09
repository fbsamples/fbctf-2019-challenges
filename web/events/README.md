Event view
========

**Challenge description:** I heard cookies and string formatting are safe in 2019?

**Run with:**

```
docker build --tag=events . [--no-cache]
docker run -p 80:80 events
```

**Solution description:**

1. Adding a new event allows you to choose if `name` or `address` should be shown. (field is named `event_important`)

2. Modify the value from `[name|address]` to `__init__.__globals__[app].config[SECRET_KEY]` (or similar) to leak `SECRET_KEY`.

3. Sign a cookie `user` with value `admin`.

4. Go to `/flag`.
