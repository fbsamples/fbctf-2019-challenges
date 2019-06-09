all:
	g++ -std=c++17 -pie -O1 -Wl,-z,relro,-z,now -fPIC raddest_db.cc raddest_db_database.cc -o raddest_db -D_FORTIFY_SOURCE=2
	strip raddest_db
	cp raddest_db dist/raddest_db

clean:
	rm dist/raddest_db
	rm core
	rm raddest_db
