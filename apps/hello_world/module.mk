#
# Main Makefile. This is basically the same as a module makefile.
#
# This Makefile should, at the very least, just include $(srctree)/tools/scripts/component_common.mk. By default, 
# this will take the sources in the src/ directory, compile them and link them into 
# lib(subdirectory_name).a in the build directory. 
#

$(warning app-layer........)
include $(srctree)/tools/scripts/module_common.mk
