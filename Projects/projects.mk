# ------------------------------------------------
# Projects Makefile
# Copyright (c) 2022, All Rights Reserved.
# @file    projects.mk
# @author  Wei-Lun Hsu
# @version 1.0
# ------------------------------------------------

ifeq ("$(PROJ)","")
## Unknown
$(error "Unknown project !!!")
else
include $(srctree)/Projects/$(PROJ)/component.mk
endif
