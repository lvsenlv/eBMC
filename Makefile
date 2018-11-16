all : 
	@$(MAKE) -C lib/build 
	@$(MAKE) -C procmonitor/
	@$(MAKE) -C test/

clean :
	@$(MAKE) clean -C lib/build/
	@$(MAKE) clean -C procmonitor/
	@$(MAKE) clean -C test/
