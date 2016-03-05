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

struct regionref {
    char r; /* region index */
    char b; /* block index (within region) */
};

typedef struct {
    char * values;
    int regions[MAX_REGIONS][5][2];
    struct regionref * regionsref;
    char sizes[MAX_REGIONS];
    char numregions;
    int w, h;
} sgr_t;

/* read a .sgr file and build a sgr_t structure
 * containing the content to use elsewhere.
 * sgr_grid_finalize must be used to free malloc'd stuff. */
sgr_t * sgr_grid_from_file(char * filename);

void sgr_grid_finalize(sgr_t * grid);

inline char getcell(sgr_t * grid, int i, int j);

inline char getposs(sgr_t * grid, int i, int j, int poss);

/* get value in region r at cell b of grid */
char sgr_get_rb(sgr_t * grid, int r, int b);

/* set value in region r at cell b of grid */
void sgr_set_rb(sgr_t * grid, int r, int b, int v);

void sgr_display(sgr_t * grid);

char sgr_checkwin(sgr_t * grid);

char sgr_checkevol(sgr_t * grid);

#endif /* __SGR_H__ */

