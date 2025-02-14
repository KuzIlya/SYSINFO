CC := gcc

SRC_CPU := sysinfo/cpu/cpu_info.c
SRC_DEVICES := sysinfo/devices/devices_info.c
SRC_MAIN := sysinfo/main.c
SRC_HOST_INFO := sysinfo/host/host_info.c
SRC_PLOT := sysinfo/plot/plot.c
SRC_GNUPLOT := sysinfo/plot/gnuplot/gnuplot.c

SRC := $(SRC_CPU) $(SRC_GNUPLOT) $(SRC_MEMORY) $(SRC_PLOT) $(SRC_DEVICES) $(SRC_HOST_INFO) $(SRC_UTILS) $(SRC_MAIN)

OUT_LINUX := sysinfo_linux
OUT_WIN := sysinfo.exe

ifeq ($(OS),Windows_NT)
    TARGET := $(OUT_WIN)
    LDLIBS := -lpdh
else
    TARGET := $(OUT_LINUX)
    LDLIBS :=
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(LDLIBS)
