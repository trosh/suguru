#include <sgr_values.h>

void sgr_remove_touching_possibilities(sgr_t * grid) {
    int i, j, ii, jj;
    char v, vv;
    /* for each cell */
    for (i=0; i<grid->h; i++)
    for (j=0; j<grid->w; j++) {
        v = grid->values[i*grid->w+j];
        /* if is final value */
        if (v > 0) {
            /* for each adjacent cell
             * (including current one, but it is final so it's ok) */
            /* for each adjacent block */
            for (ii=i-1; ii<=i+1; ii++)
            for (jj=j-1; jj<=j+1; jj++) {
                /* if in grid */
                if (ii < 0 || ii >= grid->h
                 || jj < 0 || jj >= grid->w)
                    continue;
                vv = grid->values[ii*grid->w+jj];
                /* if is not final */
                if (vv < 0) {
                    /* remove possibility */
                    vv |= 1 << v-1;
                    grid->values[ii*grid->w+jj] = vv;
                }
            }
        }
    }
}

void sgr_find_final_values(sgr_t * grid) {
    int i, j;
    char v;
    /* for each cell */
    for (i=0; i<grid->h; i++)
    for (j=0; j<grid->w; j++) {
        v = grid->values[i*grid->w+j];
        /* if not final value */
        if (v < 0) {
            /* if contains only one possibility */
            switch (v) {
                /* voodoo :-) */
                case  -2: v = 1; break;
                case  -3: v = 2; break;
                case  -5: v = 3; break;
                case  -9: v = 4; break;
                case -17: v = 5; break;
            }
            /* set as final value */
            grid->values[i*grid->w+j] = v;
        }
    }
}

void sgr_passvalues(sgr_t * grid) {
    sgr_remove_touching_possibilities(grid);
    sgr_find_final_values(grid);
}

char sgr_get_rb(sgr_t * grid, int r, int b) {
    int i, j;
    i = grid->regions[r][b][0];
    j = grid->regions[r][b][1];
    return grid->values[i*grid->w+j];
}

void sgr_set_rb(sgr_t * grid, int r, int b, int v) {
    int i, j;
    i = grid->regions[r][b][0];
    j = grid->regions[r][b][1];
    grid->values[i*grid->w+j] = v;
}

