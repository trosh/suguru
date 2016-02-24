#include <sgr.h>

#define LINE_LEN 128
#define READLINE(len) if (fgets(line, (len), file) == NULL) break;

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

