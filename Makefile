CC := g++
CFLAGS := -std=c++17 -g -O3 -mavx2 -Wall -Wextra -Wno-missing-field-initializers -I ./
LFLAGS :=
OBJDIR := obj

OBJ_FUNC = $(addprefix $(OBJDIR)/, $(1:.cpp=.o))
DEP_FUNC = $(addprefix $(OBJDIR)/, $(1:.cpp=.d))

LIBDIR  := lib
LIBSRC  := $(wildcard $(LIBDIR)/*.cpp)
LIB_OBJ := $(call OBJ_FUNC, $(LIBSRC))

POPTEST_SRC := main_poptest.cpp hashtable.cpp
POPTEST_OBJ := $(call OBJ_FUNC, $(POPTEST_SRC))
POPTEST_RUN_FILE := "run/poptest.elf"

PERFTEST_SRC := main_perfomance.cpp hashtable.cpp
PERFTEST_OBJ := $(call OBJ_FUNC, $(PERFTEST_SRC))
PERFTEST_RUN_FILE := "run/perftest.elf"



all: libs $(POPTEST_RUN_FILE) $(PERFTEST_RUN_FILE)

$(POPTEST_RUN_FILE): libs $(POPTEST_OBJ)
	$(CC) $(LIB_OBJ) $(POPTEST_OBJ) -o $@ $(LFLAGS)

$(PERFTEST_RUN_FILE): libs $(PERFTEST_OBJ)
	$(CC) $(LIB_OBJ) $(PERFTEST_OBJ) -o $@ $(LFLAGS)

libs: $(LIB_OBJ)

$(OBJDIR)/%.d : %.cpp
	mkdir -p $(dir $@)
	$(CC) $< -c -MMD -MP -o $(@:.d=.o) $(CFLAGS)

$(OBJDIR)/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CC) $< -c -MMD -MP -o $@ $(CFLAGS)

-include $(wildcard $(OBJDIR)/*.d)