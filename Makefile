eval: *.h main.cpp
	g++ -std=c++11 main.cpp -o eval 
	

bk : *.h benchmark.cpp
	g++ -std=c++11 -I/usr/local/opt/lua@5.3/include -L/usr/local/opt/lua@5.3/lib  -llua  benchmark.cpp -o bk

bk2 : *.h benchmark.cpp
	g++ -std=c++11 -I/opt/homebrew/Cellar/lua@5.3/5.3.6/include -L/opt/homebrew/Cellar/lua@5.3/5.3.6/lib  -llua  benchmark.cpp -o bk



clean: 
	-rm bk eval 
