
# conflog.make - dump of configuration info

# Create a "conf.log" file of the versions of all(?) required tools
conflog:
# Testing...
	@echo "Build settings:" >conf.log
	@echo "---------------" >>conf.log
#	@echo DRVBASE: $(DRVBASE) >>conf.log
#	@echo SRCBASE: $(SRCBASE) >>conf.log
	@echo TARGETTYPE: $(TARGETTYPE) >>conf.log
	@echo CC: $(CC) >>conf.log
	@echo AR: $(AR) >>conf.log
	@echo CFLAGS: $(CFLAGS) >>conf.log
	@echo BOARDFLAGS: $(BOARDFLAGS) >>conf.log
	@echo HOST_OS: $(HOST_OS) >>conf.log
	@echo BUILDTYPE: $(BUILDTYPE) >>conf.log
	@echo THISTARGET: $(THISTARGET) >>conf.log
	@echo >>conf.log
#	@echo ALL_SRCS: $(ALL_SRCS) >>conf.log
#	@echo >>conf.log
#	@echo OBJS: $(OBJS) >>conf.log
#	@echo >>conf.log
	@echo "=== Used tools: ===" >>conf.log
# The ST2x0 toolset might not be available on every system..
#	@echo "------------------------------------------------" >>conf.log
#	@echo "ST2x0 toolchain:" >>conf.log
#	@echo "------------------------------------------------" >>conf.log
#	@version >>conf.log
	@echo "------------------------------------------------" >>conf.log
	@echo "ST40 Toolchain:" >>conf.log
	@echo "------------------------------------------------" >>conf.log
	@sh4gcc --version >>conf.log
	@sh4as --version >>conf.log
	@sh4ld -v >>conf.log
	@sh4gdb --version >>conf.log
	@echo "------------------------------------------------" >>conf.log
ifeq ($(HOST_OS), cygwin)
	@echo "Cygwin environment:" >>conf.log
	@echo "------------------------------------------------" >>conf.log
	@cygcheck -s >>conf.log
endif
ifeq ($(HOST_OS), linux)
	@echo "Linux environment:" >>conf.log
	@echo "------------------------------------------------" >>conf.log
	@echo "Operating system:" >>conf.log
	@uname -a >>conf.log
	@echo "GNU Make:" >>conf.log
	@make --version >>conf.log
endif


