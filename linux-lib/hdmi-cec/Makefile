CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

# list of platforms which want this test case
INCLUDE_LIST:= IMX6Q

OBJ = mxc_hdmi-cec.o

LIBNAME = libcec
SONAMEVERSION=1

ifeq ($(PLATFORM), $(findstring $(PLATFORM), $(INCLUDE_LIST)))

ifeq ($(findstring IMX5, $(PLATFORM)), IMX5)
VERSION = imx5
else
VERSION = $(shell echo $(PLATFORM_STR) | awk '{print substr($$0, 1, 5)}' \
	| awk '{print  tolower($$0) }' )
endif

all: $(LIBNAME).so $(LIBNAME).a

install: install_headers
	@mkdir -p $(DEST_DIR)/usr/lib
	cp -P $(LIBNAME).* $(DEST_DIR)/usr/lib

install_headers:
	@mkdir -p $(DEST_DIR)/usr/include
	cp hdmi-cec.h $(DEST_DIR)/usr/include
else
all install :
endif

%.o: %.c
	$(CC) -D$(PLATFORM) -Wall -O2 -fPIC -c $^ -o $@

$(LIBNAME).so.$(SONAMEVERSION): $(OBJ)
	$(CC) -shared -nostartfiles -Wl,-soname,$@ $^ -o $@

$(LIBNAME).so: $(LIBNAME).so.$(SONAMEVERSION)
	ln -s $< $@

$(LIBNAME).a: $(OBJ)
	$(AR) -rc $@  $^

.PHONY: clean
clean:
	rm -f $(LIBNAME).* $(OBJ)
