


all: lib libdoc
	


lib: 
	@cd src && make

libdoc: 
	doxygen ngadmin


clean: 
	@rm -rf doc
	@cd src && make clean

mrproper: clean
	@cd src && make mrproper

