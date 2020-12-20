XBE_TITLE = pbgl_sample6
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.c $(CURDIR)/object.c
NXDK_DIR = $(CURDIR)/../../..
NXDK_PBGL = y
NXDK_SDL = y

all_local: cp_data all

include $(NXDK_DIR)/Makefile

cp_data:
	@mkdir -p $(OUTPUT_DIR)
	@cp -r data $(OUTPUT_DIR)/
