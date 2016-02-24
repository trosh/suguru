#include <sgr_regions.h>

void sgr_remove_region_possibilities(sgr_t * grid, int r) {
    int b, bb;
    char v, vv;
    /* for each final value */
    for (b=0; b<grid->sizes[r]; b++) {
        v = sgr_get_rb(grid, r, b);
        if (v > 0) {
            /* for each inner cell */
            for (bb=0; bb<grid->sizes[r]; bb++) {
                vv = sgr_get_rb(grid, r, bb);
                /* if not final value */
                if (vv < 0) {
                    /* remove possibility */
                    vv |= 1 << v-1;
                    sgr_set_rb(grid, r, bb, vv);
                }
            }
        }
    }
}

void sgr_enumerate_possibilities(
        /* input */
        sgr_t * grid, int r,
        /* output */
        char cntposs[5],
        char indposs[5][5]) {
    int b;
    char v, vv;
    bzero(cntposs, 5);
    /* for each inner cell */
    for (b=0; b<grid->sizes[r]; b++) {
        v = sgr_get_rb(grid, r, b);
        /* if not final value */
        if (v < 0) {
            for (vv=0; vv<grid->sizes[r]; vv++) {
                if (~v & 1 << vv) {
                    /* keep last block number, used to
                     * remove lonely possibilities and
                     * finding subregions.
                     * and increment possibility count */
                    indposs[vv][cntposs[vv]++] = b;
                }
            }
        }
    }
}

void sgr_find_lonely_possibilities(
        /* input/output */
        sgr_t * grid,
        /* input */
        int r,
        char cntposs[5],
        char indposs[5][5]) {
    int b;
    char vv;
    /* for each possibility counted only once */
    for (vv=0; vv<grid->sizes[r]; vv++) {
        if (cntposs[vv] == 1) {
            /* set as final value */
            b = indposs[vv][0];
            sgr_set_rb(grid, r, b, vv+1);
        }
    }
}

void sgr_find_intersections(
        /* input/output */
        sgr_t * grid,
        /* input */
        int r,
        char cntposs[5]) {
    int i, j, ii, jj, b;
    char v, vv, * map;
    map = malloc(grid->w * grid->h);
    /* for each possibility repeated more than once */
    for (vv=0; vv<grid->sizes[r]; vv++) {
        if (cntposs[vv] > 1) {
            /* work out touching possibility intersection */
            bzero(map, grid->w*grid->h);
            /* for each block containing given possibility */
            for (b=0; b<grid->sizes[r]; b++) {
                i = grid->regions[r][b][0];
                j = grid->regions[r][b][1];
                map[i*grid->w+j] = -1; /* within region */
                v = grid->values[i*grid->w+j];
                if (v < 0
                 && ~v & 1 << vv) {
                    /* for each adjacent block */
                    for (ii=i-1; ii<=i+1; ii++)
                    for (jj=j-1; jj<=j+1; jj++) {
                        /* if in map and not in region */
                        if (ii < 0 || ii >= grid->h
                         || jj < 0 || jj >= grid->w
                         || map[ii*grid->w+jj] == -1)
                            continue;
                        /* increment number of times this
                         * adjacent block was touched */
                        map[ii*grid->w+jj]++;
                    }
                }
            }
            /* for every block in map */
            for (i=0; i<grid->h; i++)
            for (j=0; j<grid->w; j++)
                /* if is touched by every inner block
                 * with given possibility and
                 * is not a final value */
                if (map[i*grid->w+j] == cntposs[vv]
                 && grid->values[i*grid->w+j] < 0) {
                    /* remove possibility */
                    grid->values[i*grid->w+j] |= 1 << vv;
                }
        }
    }
    free(map);
}

void sgr_find_subregions(
        sgr_t * grid,
        int r,
        char cntposs[5],
        char indposs[5][5]) {
    int v1, v2, v3, b;
    for (v1=0; v1<grid->sizes[r]; v1++) {
        switch (cntposs[v1]) {
          case 2:
            for (v2=0; v2<grid->sizes[r]; v2++) {
                /* search for possibility filling
                 * the same two blocks */
                if (cntposs[v2] != 2
                 || (indposs[v2][0] != indposs[v1][0]
                  && indposs[v2][0] != indposs[v1][1])
                 || (indposs[v2][1] != indposs[v1][0]
                  && indposs[v2][1] != indposs[v1][1]))
                    continue;
                /* TODO FOUND SUBREGION */
            }
            break;
          case 3:
            for (v2=0; v2<grid->sizes[r]; v2++) {
                for (v3=0; v3<grid->sizes[r]; v3++) {
                    /* TODO FIND SUBREGIONS */
            }
        }
    }
}

void sgr_passregions(sgr_t * grid) {
    int r, b, i, j, bb, ii, jj;
    char v, vv, cntposs[5], indposs[5][5];
    /* for each region */
    for (r=0; r<grid->numregions; r++) {
        sgr_remove_region_possibilities(grid, r);
        sgr_enumerate_possibilities(grid, r, cntposs, indposs);
        sgr_find_lonely_possibilities(grid, r, cntposs, indposs);
        sgr_find_intersections(grid, r, cntposs);
        sgr_find_subregions(grid, r, cntposs, indposs);
    }
}

