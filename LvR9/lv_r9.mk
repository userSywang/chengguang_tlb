LV_R9_DIR_NAME ?= LvR9

CSRCS += $(shell find -L $(LVGL_DIR)/$(LV_R9_DIR_NAME) -name "*.c")
