#include <sgr.h>

#define LINE_LEN 128

int get(sgr_t * grid, int i, int j)
{
	return grid->values[i * grid->w + j];
}

void set(sgr_t * grid, int i, int j, int v)
{
	grid->values[i * grid->w + j] = v;
}

/* TODO use this shite */
int getposs(sgr_t * grid, int i, int j, int poss)
{
	assert(poss >= 1 && poss <= MAX_BLOCKS);
	int cell = get(grid, i, j);
	assert(cell < 0);
	return ~poss & (1 << cell);
}

void _read_regions(sgr_t * grid, FILE * file)
{
	for (int i = 0; i < grid->h; ++i)
	for (int j = 0; j < grid->w; ++j) {
		int r;
		fscanf(file, "%d", &r);
		if (r < 0) {
			fprintf(stderr,
				"negative region at (%d,%d)\n", r, i, j);
			exit(1);
		}
		if (grid->sizes[r] == MAX_BLOCKS
		 || r >= MAX_REGIONS) {
			fprintf(stderr,
				"invalid region #%d at (%d,%d)\n", r, i, j);
			exit(1);
		}
		if (r >= grid->numregions) {
			grid->numregions = r + 1;
		}
		grid->regions[r][grid->sizes[r]].i = i;
		grid->regions[r][grid->sizes[r]].j = j;
		int rr = i * grid->w + j;
		grid->regionsref[rr].r = r;
		grid->regionsref[rr].b = grid->sizes[r]++;
	}
	for (int r = 0; r < grid->numregions; ++r)
	for (int b = 0; b < grid->sizes[r]; ++b) {
		int i = grid->regions[r][b].i;
		int j = grid->regions[r][b].j;
		set(grid, i, j, - (1 << grid->sizes[r]));
	}
}

void _read_values(sgr_t * grid, FILE * file)
{
	int linetab[LINE_LEN];
	char * line = (char *)linetab;
	for (int i = 0; i < grid->h; ++i) {
		//if (fgets(line, LINE_LEN, file) == NULL) {
		//	fprintf(stderr,
		//		"file ended before reading all values\n");
		//	exit(1);
		//}
		for (int j = 0; j < grid->w; ++j) {
			int v = getc(file) - '0';
			if (v < 0
			 || v > MAX_BLOCKS) {
				fprintf(stderr,
					"invalid value %c at (%d,%d)\n",
					i, j, line[j]);
				exit(1);
			}
			if (v)
				set(grid, i, j, v);
		}
		if (getc(file) != '\n') {
			fprintf(stderr,
				"expecting newline at line %d\n",
				i);
			exit(1);
		};
	}
}

sgr_t * sgr_grid_from_file(char * filename)
{
	FILE * file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr,
			"error opening %s\n", filename);
		exit(1);
	}
	char linetab[LINE_LEN];
	char * line = (char*)linetab;
	sgr_t * grid = malloc(sizeof(sgr_t)); /* freed in sgr_grid_finalize */
	grid->w = 0;
	grid->h = 0;
	grid->values = NULL;
	bzero(grid->sizes, sizeof(int) * MAX_REGIONS);
	grid->numregions = 0;
	int filled_regions = 0;
	while (fgets(line, LINE_LEN, file) != NULL) {
		int onlyspaces = 1;
		for (int i = 0; line[i]; ++i) {
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
			grid->values = malloc(sizeof(int) * grid->w * grid->h);
			grid->regionsref = malloc(
				grid->w * grid->h * sizeof(struct regionref));
		}
		if (grid->values != NULL
		 && !strncmp(line, "regions", 7)) {
			_read_regions(grid, file);
			filled_regions = 1;
			continue;
		}
		if (filled_regions && !strncmp(line, "values", 6)) {
			_read_values(grid, file);
			continue;
		}
	}
	fclose(file);
	return grid;
}

void sgr_grid_finalize(sgr_t * grid)
{
	free(grid->values);
	free(grid->regionsref);
	free(grid);
}

void sgr_display(sgr_t * grid)
{
	for (int i = 0; i < grid->h; ++i) {
		for (int j = 0; j < grid->w; ++j) {
			int r = grid->regionsref[i * grid->w + j].r;
			//int b = grid->regionsref[i * grid->w + j].b;
			int v = grid->values[i * grid->w + j];
			printf("\033[4%d;9%dm%c \033[0m",
				r % 7, r / 7,
				v >= 0 ? v + '0' : ' ');
		}
		putchar('\n');
	}
}

int sgr_checkwin(sgr_t * grid)
{
	for (int i = 0; i < grid->w * grid->h; ++i)
		if (grid->values[i] < 0)
			return 0;
	return 1;
}

int sgr_checkevol(sgr_t * grid)
{
	static int prevchecksum = INT_MIN;
	int checksum = 0;
	for (int i = 0; i < grid->w * grid->h; ++i)
		if (grid->values[i] < 0)
			checksum += grid->values[i];
	int evol = prevchecksum != checksum;
	prevchecksum = checksum;
	return evol;
}
