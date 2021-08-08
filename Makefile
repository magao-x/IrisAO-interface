
allall: all

OTHER_HEADERS=
TARGET=shmirisAO

IRISAO_PATH=/opt/IrisAO
LDFLAGS += -L$(IRISAO_PATH) -Wl,-rpath,$(IRISAO_PATH)
LDLIBS+= -lirisao.devices.1.0.2.5 -lImageStreamIO
INCLUDES+= -I/$(IRISAO_PATH)
