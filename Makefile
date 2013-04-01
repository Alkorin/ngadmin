

all: raw lib libdoc cli
	


raw: force
	@+$(MAKE) -C raw lib


lib: raw force
	@+$(MAKE) -C lib lib


libdoc: force
	@+$(MAKE) -C lib libdoc


cli: lib force
	@+$(MAKE) -C cli


dump: raw force
	@+$(MAKE) -C dump


force: 
	@true


clean:
	@+$(MAKE) -C raw clean
	@+$(MAKE) -C lib clean
	@+$(MAKE) -C cli clean
	@+$(MAKE) -C dump clean

mrproper: clean
	@+$(MAKE) -C raw mrproper
	@+$(MAKE) -C lib mrproper
	@+$(MAKE) -C cli mrproper
	@+$(MAKE) -C dump mrproper


