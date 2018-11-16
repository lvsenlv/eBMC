COMMON_DIR = ../../inc

all : 
	@$(MAKE) -f errno.mk  COMMON=$(COMMON_DIR) 
	@$(MAKE) -f crc.mk    COMMON=$(COMMON_DIR) 
	@$(MAKE) -f file.mk   COMMON=$(COMMON_DIR) 
	@$(MAKE) -f hash.mk   COMMON=$(COMMON_DIR) 
	@$(MAKE) -f parser.mk COMMON=$(COMMON_DIR) 
	@$(MAKE) -f queue.mk  COMMON=$(COMMON_DIR) 
	@$(MAKE) -f log.mk  COMMON=$(COMMON_DIR) 

clean :
	@$(MAKE) clean -f errno.mk
	@$(MAKE) clean -f crc.mk
	@$(MAKE) clean -f file.mk
	@$(MAKE) clean -f hash.mk
	@$(MAKE) clean -f parser.mk
	@$(MAKE) clean -f queue.mk
	@$(MAKE) clean -f log.mk
