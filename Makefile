eval: *.h main.cpp
	g++ -std=c++11 main.cpp -o eval 
	

bk : *.h benchmark.cpp
	g++ -std=c++11 benchmark.cpp -o bk

clean: 
	-rm bk eval 
