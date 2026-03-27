ifeq ($(ewokos),)
$(error ewokos dir must set ewokos=[dir])
endif

all: 
	mkdir -p build
	cd mariox; make

clean:	
	cd mariox; make clean
	rm -fr build
