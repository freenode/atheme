# Copyright (c) 2003-2004 E. Will et al.
# Rights to this code are documented in doc/LICENSE.
#
# This file contains build instructions.
#
#

MODULE = spqf

SRCS = opine.c kick.c quiet.c clear.c setlock.c setdef.c set.c info.c list.c vote.c help.c main.c

include ../../extra.mk
include ../../buildsys.mk
include ../../buildsys.module.mk

CPPFLAGS += -I../../include -x c++ -std=gnu++11 -fno-rtti -s -fmax-errors=1
LIBS +=	-L../../libathemecore -lathemecore ${LDFLAGS_RPATH} -lstdc++

