
all: 
	mkdir -p build
	cd mariox; make

clean:	
	cd mariox; make clean
	rm -fr build
