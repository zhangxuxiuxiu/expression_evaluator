eval: *.h main.cpp
	g++ -std=c++11 main.cpp -o eval 
	

bk : *.h benchmark.cpp
	g++ -std=c++11 benchmark.cpp -o bk

lua : lua_test.cpp
	g++ -I/usr/local/opt/lua@5.3/include -L/usr/local/opt/lua@5.3/lib  -llua lua_test.cpp -o lt

clean: 
	-rm bk eval 
