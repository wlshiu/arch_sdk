#
# component Makefile
#


###
# Add linker arguments to the LDFLAGS for the app executable. Defaults to -l$(COMPONENT_NAME).
# If adding pre-compiled libraries to this directory, add them as absolute paths - ie $(COMPONENT_PATH)/libwhatever.a
COMPONENT_ADD_LDFLAGS +=

###
# Paths, relative to the component directory, which will be added to the include search path for all components in the project.
# Defaults to include if not overridden.
# If an include directory is only needed to compile this specific component, add it to COMPONENT_PRIV_INCLUDEDIRS instead.
COMPONENT_ADD_INCLUDEDIRS += include

###
# Directory paths, must be relative to the component directory,
# which will be added to the include search path for this component's source files only.
COMPONENT_PRIV_INCLUDEDIRS += include

###
# Object files to compile. Default value is a .o file for each source file that is found in COMPONENT_SRCDIRS.
# Overriding this list allows you to exclude source files in COMPONENT_SRCDIRS that would otherwise be compiled.
COMPONENT_OBJS := \
    utils_log.o \
    utils_printf.o \
    shell.o \
    utils.o

###
# Directory paths, must be relative to the component directory, which will be searched for source files (*.cpp, *.c, *.S).
# Defaults to '.', ie the component directory itself.
# Override this to specify a different list of directories which contain source files.
COMPONENT_SRCDIRS +=

###
# set CFLAGS
CFLAGS +=

###
# set CXXFLAGS
CXXFLAGS +=

###
# set LDLAGS
LDLAGS +=
