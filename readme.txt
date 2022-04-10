read main.cpp for usages, read benckmark.cpp for script expression examples.

benchmark:
	environment:  macos catalina 10.15.7,  2.5GHZ, 8 core i9, 1M ops
		raw parsed function took 12780ms
		lua parsed function took 736ms
		ast parsed function took 575ms
		vm parsed function took 527ms
		raw cpp function took 64ms
