ifeq ($(ewokos),)
ewokos=../ewokos
endif

ifeq ($(ARCH),)
export ARCH=aarch64
endif

ifeq ($(HW),)
export HW=virt
endif

all: 
	mkdir -p build
	cd mariox; make
	cp -r data $(ewokos)/system/build/$(HW)/rootfs/

clean:	
	cd mariox; make clean
	rm -fr build
