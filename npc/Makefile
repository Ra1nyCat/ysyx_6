TOPNAME=top

NXDC_FILES=/home/yanxy/ysyx/ysyx-workbench/nvboard/example/constr/top.nxdc

INC_PATH?=

VERILATOR=verilator

VERILATOR_FLAGS=-Wall -cc --trace --exe --build

BUILD_DIR=./example/build

OBJ_DIR=$(BUILD_DIR)/obj_dir

BIN=$(BUILD_DIR)/$(TOPNAME)

$(shell mkdir -p $(BUILD_DIR))

#constrint file

#project source
VSRCS=$(shell find $(abspath ./example/vsrc) -name "*.v")
CSRCS=$(shell find $(abspath ./example/csrc) -name "*.cpp" -or -name "*.c" -or -name "*.cc")

#rules for verilator
INCFLAGS=$(addprefix -I,$(INC_PATH))
CFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
LDFLAGS += -lSDL2 -lSDL2_image


$(BIN):$(VSRCS) $(CSRCS)
	@rm -rf $(OBJ_DIR)
	$(info "----------------")
	$(info $(CSRCS))
	$(info "----------------")
	$(VERILATOR) $(VERILATOR_FLAGS) \
	--top-module $(TOPNAME) $^ \
	$(addprefix -CFLAGS ,$(CFLAGS)) \
	--Mdir $(OBJ_DIR) \
	--exe -o $(abspath $(BIN))

clean:
	rm -rf $(BUILD_DIR)

.PHONY:clean sim all default

all:
	@echo "Write this Makefile by your self."

sim:$(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$^

include ../Makefile
