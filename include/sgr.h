#ifndef __SGR_H__
#define __SGR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/* TODO use this storage instead of directly working
 * with char values */
union cell {
	unsigned char final_value : 3;
	struct {
		unsigned char one       : 1;
		unsigned char two       : 1;
		unsigned char three     : 1;
		/* one, two and three are overwritable by final_value */
		unsigned char four      : 1;
		unsigned char five      : 1;
		/* union choice bit, cannot be affected by final_value */
		unsigned char not_final : 1;
	} choices;
};

#define MAX_REGIONS 64
#define MAX_BLOCKS 10

struct regionref {
	int r; /* region index */
	int b; /* block index (within region) */
};

struct coords {
	int i; /* row */
	int j; /* column */
};

typedef struct {
	int * values;
	struct coords regions[MAX_REGIONS][MAX_BLOCKS];
	struct regionref * regionsref;
	int sizes[MAX_REGIONS];
	int numregions;
	int w, h;
} sgr_t;

/* read a .sgr file and build a sgr_t structure
 * containing the content to use elsewhere.
 * sgr_grid_finalize must be used to free malloc'd stuff. */
sgr_t * sgr_grid_from_file(char * filename);

void sgr_grid_finalize(sgr_t * grid);

void sgr_display(sgr_t * grid);

int sgr_checkwin(sgr_t * grid);

int sgr_checkevol(sgr_t * grid);

#endif /* __SGR_H__ */
