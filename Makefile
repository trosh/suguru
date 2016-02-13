CC := gcc
CFLAGS := -O2
TARGET := suguru
GRID := grids/s_v1_b16_3.sgr

.PHONY: all run clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $< -o $@ $(CFLAGS)

run: $(TARGET)
	./$< $(GRID)

clean:
	rm -f $(TARGET)

