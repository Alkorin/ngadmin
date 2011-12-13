

all: lib libdoc cli
	


lib: force
	@cd lib && make lib


libdoc: force
	@cd lib && make libdoc


cli: force
	@cd cli && make



force: 
	@true


clean:
	@cd lib && make clean
	@cd cli && make clean

mrproper: clean
	@cd lib && make mrproper
	@cd cli && make mrproper


