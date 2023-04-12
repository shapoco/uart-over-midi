.PHONY: all install clean

REPO_DIR=$(shell pwd)
SRC_DIR=src
BUILD_DIR=build

BIN_NAME=pico_hub75.uf2
BIN=$(BUILD_DIR)/$(BIN_NAME)

SRC_LIST=$(wildcard $(src)/*.*)

all: $(BIN)

$(BIN): $(SRC_LIST) CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) \
		&& cmake .. \
		&& make -j
	@echo "------------------------------"
	@echo "UF2 File:"
	@echo $(REPO_DIR)/$(BIN)
	@ls -l $(REPO_DIR)/$(BIN)

install: $(BIN)
	sudo mkdir -p /mnt/e
	sudo mount -t drvfs e: /mnt/e
	cp $(BIN) /mnt/e/.

clean:
	rm -rf build

serve:
	echo "http://localhost:8080/"
	cd html; python3 -m http.server 8080
