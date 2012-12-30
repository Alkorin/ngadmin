

all: lib libdoc cli
	


lib: force
	@+$(MAKE) -C lib lib


libdoc: force
	@+$(MAKE) -C lib libdoc


cli: force
	@+$(MAKE) -C cli



force: 
	@true


clean:
	@+$(MAKE) -C lib clean
	@+$(MAKE) -C cli clean

mrproper: clean
	@+$(MAKE) -C lib mrproper
	@+$(MAKE) -C cli mrproper


