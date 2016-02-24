CC := gcc
CFLAGS := -O2 -g -Iinclude
TARGET := suguru
GRID := grids/s_v1_b16_3.sgr

.PHONY: all run clean

all: $(TARGET)

$(TARGET): main.o sgr.o sgr_values.o sgr_regions.o sgr_solve.o \
           include/sgr.h         include/sgr_values.h \
           include/sgr_regions.h include/sgr_solve.h
	$(CC) sgr.o sgr_values.o sgr_regions.o sgr_solve.o $< \
	      -o $@ $(CFLAGS)

main.o: src/main.c include/sgr.h include/sgr_solve.h
	$(CC) $< -c -o $@ $(CFLAGS)

sgr.o: src/sgr.c include/sgr.h
	$(CC) $< -c -o $@ $(CFLAGS)

sgr_values.o: src/sgr_values.c include/sgr_values.h include/sgr.h
	$(CC) $< -c -o $@ $(CFLAGS)

sgr_regions.o: src/sgr_regions.c include/sgr_regions.h include/sgr.h
	$(CC) $< -c -o $@ $(CFLAGS)

sgr_solve.o: src/sgr_solve.c include/sgr_solve.h include/sgr.h
	$(CC) $< -c -o $@ $(CFLAGS)

run: $(TARGET)
	./$< $(GRID)

clean:
	rm -f $(TARGET) *.o

