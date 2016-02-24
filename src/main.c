#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

sgr_t * sgr_readfile(char * filename) {
    FILE * file;
    char linetab[LINE_LEN];
    char * line;
    sgr_t * grid;
    int i, j, r, b;
    char v;
    int filled_regions;
    file = fopen(filename, "r");
    line = (char*)linetab;
    grid = malloc(sizeof(sgr_t));
    grid->w = 0;
    grid->h = 0;
    grid->values = NULL;
    bzero(grid->sizes, MAX_REGIONS);
    grid->numregions = 0;
    filled_regions = 0;
    while (1) {
        READLINE(LINE_LEN);
        for (i=0; line[i]; i++) {
            if (line[i] != ' '
             && line[i] != '\t'
             && line[i] != '\n')
                goto nonspace;
        }
        continue;
nonspace:
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

void sgr_passvalues(sgr_t * grid) {
    int i, j, ii, jj;
    char v, vv;
    for (i=0; i<grid->h; i++)
    for (j=0; j<grid->w; j++) {
        v = grid->values[i*grid->w+j];
        if (v > 0) {
            for (ii  = (i-1 < 0 ? 0 : i-1);
                 ii  < ((i+1 >= grid->h) ? grid->h : i+2);
                 ii += 1)
            for (jj  = (j-1 < 0 ? 0 : j-1);
                 jj  < ((j+1 >= grid->w) ? grid->w : j+2);
                 jj += 1) {
                if (ii == i
                 && jj == j)
                    continue;
                vv = grid->values[ii*grid->w+jj];
                if (vv < 0) {
                    vv |= 1<<v-1;
                    grid->values[ii*grid->w+jj] = vv;
                }
            }
        }
    }
    for (i=0; i<grid->h; i++)
    for (j=0; j<grid->w; j++) {
        v = grid->values[i*grid->w+j];
        if (v < 0) {
            switch (v) {
                case  -2: v = 1; break;
                case  -3: v = 2; break;
                case  -5: v = 3; break;
                case  -9: v = 4; break;
                case -17: v = 5; break;
            }
            grid->values[i*grid->w+j] = v;
        }
    }
}

//char sgr_rb2v(sgr_t * grid, int r, int b) {
//    int i, j;
//    i = grid->regions[r][b][0];
//    j = grid->regions[r][b][1];
//    return grid->values[i*grid->w+j];
//}

void sgr_passregions(sgr_t * grid) {
    int r, b, i, j, v, bb, ii, jj, vv;
    char cntposs[5], indposs[5], *map;
    map = malloc(grid->w*grid->h);
    for (r=0; r<grid->numregions; r++) {
        for (b=0; b<grid->sizes[r]; b++) {
            i = grid->regions[r][b][0];
            j = grid->regions[r][b][1];
            v = grid->values[i*grid->w+j];
            if (v > 0) {
                for (bb=0; bb<grid->sizes[r]; bb++) {
                    ii = grid->regions[r][bb][0];
                    jj = grid->regions[r][bb][1];
                    vv = grid->values[ii*grid->w+jj];
                    if (vv < 0) {
                        vv |= 1<<v-1;
                        grid->values[ii*grid->w+jj] = vv;
                    }
                }
            }
        }
        bzero(cntposs, 5);
        for (b=0; b<grid->sizes[r]; b++) {
            i = grid->regions[r][b][0];
            j = grid->regions[r][b][1];
            v = grid->values[i*grid->w+j];
            if (v < 0) {
                for (vv=0; vv<grid->sizes[r]; vv++) {
                    if ((~v) & (1 << vv)) {
                        cntposs[vv]++;
                        indposs[vv] = b;
                    }
                }
            }
        }
        for (vv=0; vv<grid->sizes[r]; vv++) {
            if (cntposs[vv] == 1) {
                b = indposs[vv];
                i = grid->regions[r][b][0];
                j = grid->regions[r][b][1];
                grid->values[i*grid->w+j] = vv+1;
            } else if (cntposs[vv] > 1) {
                bzero(map, grid->w*grid->h);
                for (b=0; b<grid->sizes[r]; b++) {
                    i = grid->regions[r][b][0];
                    j = grid->regions[r][b][1];
                    map[i*grid->w+j] = -1;
                    v = grid->values[i*grid->w+j];
                    if ((v < 0) &&
                     ((~v) & (1 << vv))) {
                        for (ii=i-1; ii<=i+1; ii++)
                        for (jj=j-1; jj<=j+1; jj++) {
                            if (ii < 0 || ii >= grid->h
                             || jj < 0 || jj >= grid->w
                             || map[ii*grid->w+jj] == -1)
                                continue;
                            map[ii*grid->w+jj]++;
                        }
                    }
                }
                for (i=0; i<grid->h; i++)
                for (j=0; j<grid->w; j++)
                    if (map[i*grid->w+j] == cntposs[vv]
                     && grid->values[i*grid->w+j] < 0
                     && (~grid->values[i*grid->w+j]) & 1<<vv) {
                        grid->values[i*grid->w+j] |= 1 << vv;
                    }
            }
        }
    }
    free(map);
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

int main(int argc, char * argv[]) {
    sgr_t * grid;
    int i;
    if (argc != 2) {
        fputs("usage: suguru FILE.sgr", stderr);
        return 1;
    }
    grid = sgr_readfile(argv[1]);
    i = 0;
    do {
        sgr_passvalues(grid);
        sgr_passregions(grid);
        i++;
    } while (sgr_checkwin(grid) == 0 && i<5000);
    sgr_display(grid);
    printf("%d iterations\n", i);
    free(grid->values); /* ? */
    return 0;
}

