CC := g++
CFLAGS := -std=c++17 -g -O3 -mavx2 -Wall -Wextra -Wno-missing-field-initializers -Wno-parentheses -Wno-unused-but-set-variable  -I ./
LFLAGS :=
OBJDIR := obj

OBJ_FUNC = $(addprefix $(OBJDIR)/, $(1:.cpp=.o))
DEP_FUNC = $(addprefix $(OBJDIR)/, $(1:.cpp=.d))

LIBDIR  := lib
LIBSRC  := $(wildcard $(LIBDIR)/*.cpp)
LIB_OBJ := $(call OBJ_FUNC, $(LIBSRC))

PERFTEST_V0_DIR 	 := v0
PERFTEST_V1_DIR 	 := v1_avx
PERFTEST_V2_DIR 	 := v2_align
PERFTEST_V3_DIR 	 := v3_nolen
PERFTEST_V7_DIR 	 := v7_final

PERFTEST_GNU_MAIN 	  := main_perf_gnu.cpp
PERFTEST_CRC_MAIN 	  := main_perf_crc.cpp
PERFTEST_CRC_U16_MAIN := main_perf_crc_u16.cpp

POPTEST_SRC := main_poptest.cpp $(wildcard $(PERFTEST_V0_DIR)/*.cpp)

PERFTEST_V0_SRC := $(PERFTEST_GNU_MAIN) $(wildcard $(PERFTEST_V0_DIR)/*.cpp)
PERFTEST_V1_SRC := $(PERFTEST_GNU_MAIN) $(wildcard $(PERFTEST_V1_DIR)/*.cpp)
PERFTEST_V2_SRC := $(PERFTEST_GNU_MAIN) $(wildcard $(PERFTEST_V2_DIR)/*.cpp)
PERFTEST_V3_SRC := $(PERFTEST_GNU_MAIN) $(wildcard $(PERFTEST_V3_DIR)/*.cpp)
PERFTEST_V4_SRC := $(PERFTEST_CRC_MAIN) $(wildcard $(PERFTEST_V3_DIR)/*.cpp)
PERFTEST_V5_SRC := $(PERFTEST_CRC_U16_MAIN) $(wildcard $(PERFTEST_V3_DIR)/*.cpp)
PERFTEST_V7_SRC := $(wildcard $(PERFTEST_V7_DIR)/*.cpp)

PERFTEST_V0_RUN_FILE := "run/perftest_$(PERFTEST_V0_DIR)"
PERFTEST_V1_RUN_FILE := "run/perftest_$(PERFTEST_V1_DIR)"
PERFTEST_V2_RUN_FILE := "run/perftest_$(PERFTEST_V2_DIR)"
PERFTEST_V3_RUN_FILE := "run/perftest_$(PERFTEST_V3_DIR)"
PERFTEST_V4_RUN_FILE := "run/perftest_v4"
PERFTEST_V5_RUN_FILE := "run/perftest_v5"
PERFTEST_V7_RUN_FILE := "run/perftest_v7_final"

PERFTEST_V7_OBJ := $(call OBJ_FUNC, $(PERFTEST_V7_SRC))

all: libs poptest v0 v1 v2 v3 $(PERFTEST_V4_RUN_FILE) $(PERFTEST_V5_RUN_FILE) $(PERFTEST_V7_RUN_FILE)

poptest: $(POPTEST_RUN_FILE)

v0: $(PERFTEST_V0_RUN_FILE)

v1: $(PERFTEST_V1_RUN_FILE)

v2: $(PERFTEST_V2_RUN_FILE)

v3: $(PERFTEST_V3_RUN_FILE)

v4: $(PERFTEST_V4_RUN_FILE)

v5: $(PERFTEST_V5_RUN_FILE)

v6: $(PERFTEST_V6_RUN_FILE)

v7: $(PERFTEST_V7_RUN_FILE)


$(POPTEST_RUN_FILE): libs
	$(CC) $(LIB_OBJ) $(POPTEST_SRC) -o $@ $(CFLAGS) -I $(PERFTEST_V0_DIR)


$(PERFTEST_V0_RUN_FILE): libs
	$(CC) $(LIB_OBJ) $(PERFTEST_V0_SRC) -o $@ $(CFLAGS) -I ./$(PERFTEST_V0_DIR)/

$(PERFTEST_V1_RUN_FILE): libs
	$(CC) $(LIB_OBJ) $(PERFTEST_V1_SRC) -o $@ $(CFLAGS) -I ./$(PERFTEST_V1_DIR)/

$(PERFTEST_V2_RUN_FILE): libs
	$(CC) $(LIB_OBJ) $(PERFTEST_V2_SRC) -o $@ $(CFLAGS) -I ./$(PERFTEST_V2_DIR)/

$(PERFTEST_V3_RUN_FILE): libs
	$(CC) $(LIB_OBJ) $(PERFTEST_V3_SRC) -o $@ $(CFLAGS) -I ./$(PERFTEST_V3_DIR)/

$(PERFTEST_V4_RUN_FILE): libs
	$(CC) $(LIB_OBJ) $(PERFTEST_V4_SRC) -o $@ $(CFLAGS) -I ./$(PERFTEST_V3_DIR)/

$(PERFTEST_V5_RUN_FILE): libs
	$(CC) $(LIB_OBJ) $(PERFTEST_V5_SRC) -o $@ $(CFLAGS) -I ./$(PERFTEST_V3_DIR)/

$(PERFTEST_V7_RUN_FILE): libs $(PERFTEST_V7_OBJ)
	$(CC) $(LIB_OBJ) $(PERFTEST_V7_OBJ) -o $@ $(LFLAGS)



libs: $(LIB_OBJ)

$(OBJDIR)/%.d : %.cpp
	mkdir -p $(dir $@)
	$(CC) $< -c -MMD -MP -o $(@:.d=.o) $(CFLAGS)

$(OBJDIR)/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CC) $< -c -MMD -MP -o $@ $(CFLAGS)

-include $(wildcard $(OBJDIR)/*.d)