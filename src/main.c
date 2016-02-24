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

#define LINE_LEN 128
#define READLINE(len) if (fgets(line, (len), file) == NULL) break;

/* read a .sgr file and build a sgr_t structure
 * containing the content to use elsewhere.
 * sgr_grid_finalize must be used to free malloc'd stuff. */
sgr_t * sgr_grid_from_file(char * filename) {
    FILE * file;
    char linetab[LINE_LEN];
    char * line;
    sgr_t * grid;
    int i, j, r, b;
    char v, onlyspaces;
    int filled_regions;
    file = fopen(filename, "r");
    line = (char*)linetab;
    grid = malloc(sizeof(sgr_t)); /* freed in sgr_grid_finalize */
    grid->w = 0;
    grid->h = 0;
    grid->values = NULL;
    bzero(grid->sizes, MAX_REGIONS);
    grid->numregions = 0;
    filled_regions = 0;
    while (1) {
        READLINE(LINE_LEN);
        onlyspaces = 1;
        for (i=0; line[i]; i++) {
            if (line[i] != ' '
             && line[i] != '\t'
             && line[i] != '\n')
                onlyspaces = 0;
        }
        if (onlyspaces)
            continue;
        /* disregard comments */
        if (line[0] == '#')
            continue;
        if (!(grid->w || strncmp(line, "width", 5))) {
            fscanf(file, "%d", &(grid->w));
            continue;
        }
        if (!(grid->h || strncmp(line, "height", 6))) {
            fscanf(file, "%d", &(grid->h));
            continue;
        }
        if (grid->values == NULL && grid->h && grid->w) {
            grid->values = malloc(grid->w*grid->h);
            grid->regionsref = malloc(grid->w*grid->h
                                    * sizeof(struct regionref));
        }
        if (grid->values != NULL
         && !strncmp(line, "regions", 7)) {
            for (i=0; i<grid->h; i++)
            for (j=0; j<grid->w; j++) {
                fscanf(file, "%d", &v);
                if (v < 0) {
                    fprintf(stderr, "negative region at (%d,%d)\n",
                            i, j);
                    exit(1);
                }
                if (grid->sizes[v] == 5) {
                    fprintf(stderr, "invalid region #%d at (%d,%d)\n",
                            v, i, j);
                    exit(1);
                }
                if (v >= grid->numregions) {
                    grid->numregions = v+1;
                }
                grid->regions[v][grid->sizes[v]][0] = i;
                grid->regions[v][grid->sizes[v]][1] = j;
                grid->regionsref[i*grid->w+j].r = v;
                grid->regionsref[i*grid->w+j].b = grid->sizes[v]++;
            }
            for (r=0; r<grid->numregions; r++)
            for (b=0; b<grid->sizes[r]; b++) {
                i = grid->regions[r][b][0];
                j = grid->regions[r][b][1];
                grid->values[i*grid->w+j] = - (1<<grid->sizes[r]);
            }
            filled_regions = 1;
            continue;
        }
        if (filled_regions && !strncmp(line, "values", 6)) {
            for (i=0; i<grid->h; i++) {
                READLINE(LINE_LEN);
                for (j=0; j<grid->w; j++) {
                    if (line[j] < '0'
                     || line[j] > '5') {
                        fprintf(stderr, "invalid value %c at (%d,%d)\n",
                                i, j, line[j]);
                        exit(1);
                    }
                    v = line[j]-'0';
                    if (v)
                        grid->values[i*grid->w+j] = v;
                }
            }
            continue;
        }
    }
    fclose(file);
    return grid;
}

void sgr_grid_finalize(sgr_t * grid) {
    free(grid->values);
    free(grid->regionsref);
    free(grid);
}

/* TODO use this shite */
inline char getcell(sgr_t * grid, int i, int j) {
    return grid->values[i*grid->w+j];
}

/* TODO use this shite */
inline char getposs(sgr_t * grid, int i, int j, int poss) {
    assert(poss >= 1 && poss <= 5);
    char cell = getcell(grid, i, j);
    assert(cell < 0);
    return ~poss & (1 << cell);
}

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

/* get value in region r at cell b of grid */
char sgr_get_rb(sgr_t * grid, int r, int b) {
    int i, j;
    i = grid->regions[r][b][0];
    j = grid->regions[r][b][1];
    return grid->values[i*grid->w+j];
}

/* set value in region r at cell b of grid */
void sgr_set_rb(sgr_t * grid, int r, int b, int v) {
    int i, j;
    i = grid->regions[r][b][0];
    j = grid->regions[r][b][1];
    grid->values[i*grid->w+j] = v;
}

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

/* In a given region, count the occurrences of each
 * possibility, and the last index of each. */
void sgr_enumerate_possibilities(
        /* input */
        sgr_t * grid, int r,
        /* output */
        char cntposs[5], char indposs[5]) {
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
                    /* increment possibility count */
                    cntposs[vv]++;
                    /* keep last block number, useful if
                     * possibility is only available once */
                    indposs[vv] = b;
                }
            }
        }
    }
}

/* Uses cntposs and indposs, generated from
 * sgr_enumerate_possibilities. */
void sgr_find_lonely_possibilities(
        /* input/output */
        sgr_t * grid,
        /* input */
        int r,
        char cntposs[5],
        char indposs[5]) {
    int b;
    char vv;
    /* for each possibility counted only once */
    for (vv=0; vv<grid->sizes[r]; vv++) {
        if (cntposs[vv] == 1) {
            /* set as final value */
            b = indposs[vv];
            sgr_set_rb(grid, r, b, vv+1);
        }
    }
}

/* For a given region, look for blocks (outside of it)
 * that touch every block within the region containing
 * a possibility, then remove that possibility from
 * the external block.
 * Uses cntposs and indposs, generated from
 * sgr_enumerate_possibilities. */
void sgr_find_intersections(
        /* input/output */
        sgr_t * grid,
        /* input */
        int r, char cntposs[5], char indposs[5]) {
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

void sgr_passregions(sgr_t * grid) {
    int r, b, i, j, bb, ii, jj;
    char v, vv, cntposs[5], indposs[5];
    /* for each region */
    for (r=0; r<grid->numregions; r++) {
        sgr_remove_region_possibilities(grid, r);
        sgr_enumerate_possibilities(grid, r, cntposs, indposs);
        sgr_find_intersections(grid, r, cntposs, indposs);
    }
}

void sgr_display(sgr_t * grid) {
    int r, b, i, j, v;
    for (i=0; i<grid->h; i++) {
        for (j=0; j<grid->w; j++) {
            r = grid->regionsref[i*grid->w+j].r;
            b = grid->regionsref[i*grid->w+j].b;
            v = grid->values[i*grid->w+j];
            printf("\033[4%d;9%dm%c \033[0m",
                   r%7, r/7, v>=0?v+'0':' ');
        }
        putchar('\n');
    }
    putchar('\n');
}

char sgr_checkwin(sgr_t * grid) {
    int i;
    for (i=0; i<grid->w*grid->h; i++)
        if (grid->values[i] < 0)
            return 0;
    return 1;
}

char sgr_checkevol(sgr_t * grid) {
    int checksum, i;
    static int prevchecksum = INT_MAX;
    checksum = 0;
    for (i=0; i<grid->w*grid->h; i++)
        if (grid->values[i] < 0)
            checksum -= grid->values[i];
    i = prevchecksum - checksum;
    prevchecksum = checksum;
    return i;
}

void sgr_solve(sgr_t * grid) {
    do {
        sgr_passvalues(grid);
        sgr_passregions(grid);
    } while (sgr_checkwin(grid) == 0
          && sgr_checkevol(grid) != 0);
}

int main(int argc, char * argv[]) {
    sgr_t * grid;
    if (argc != 2) {
        fputs("usage: suguru FILE.sgr", stderr);
        return 1;
    }
    grid = sgr_grid_from_file(argv[1]);
    sgr_solve(grid);
    sgr_display(grid);
    printf("%d iterations\n", i);
    sgr_grid_finalize(grid);
    return 0;
}

