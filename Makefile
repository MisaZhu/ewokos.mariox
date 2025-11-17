
all: 
	mkdir -p build
	cd mariox; make

clean:	
	cd apps; make clean
	rm -fr build
