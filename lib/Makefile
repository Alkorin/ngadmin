


all: lib libdoc
	


lib: 
	@+$(MAKE) -C src

libdoc: 
	doxygen ngadmin


clean: 
	@rm -rf doc
	@+$(MAKE) -C src clean

mrproper: clean
	@+$(MAKE) -C src mrproper

