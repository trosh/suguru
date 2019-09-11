#include <sgr_regions.h>
#include <sgr_values.h>

static void sgr_remove_region_possibilities(sgr_t * grid, int r)
{
	/* for each final value */
	for (int b = 0; b < grid->sizes[r]; ++b) {
		int v = sgr_get_rb(grid, r, b);
		if (v > 0) {
			/* for each inner cell */
			for (int bb = 0; bb < grid->sizes[r]; ++bb) {
				int vv = sgr_get_rb(grid, r, bb);
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
static void sgr_enumerate_possibilities(
		/* input */
		sgr_t * grid, int r,
		/* output */
		int cntposs[MAX_BLOCKS], int indposs[MAX_BLOCKS])
{
	bzero(cntposs, sizeof(int) * MAX_BLOCKS);
	bzero(indposs, sizeof(int) * MAX_BLOCKS);
	/* for each inner cell */
	for (int b = 0; b < grid->sizes[r]; ++b) {
		int v = sgr_get_rb(grid, r, b);
		/* if not final value */
		if (v < 0) {
			for (int vv = 0; vv < grid->sizes[r]; ++vv) {
				if (~v & (1 << vv)) {
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
static void sgr_find_lonely_possibilities(
		/* input/output */
		sgr_t * grid,
		/* input */
		int r,
		int cntposs[MAX_BLOCKS],
		int indposs[MAX_BLOCKS])
{
	/* for each possibility counted only once */
	for (int vv = 0; vv < grid->sizes[r]; ++vv) {
		if (cntposs[vv] == 1) {
			/* set as final value */
			int b = indposs[vv];
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
static void sgr_find_intersections(
		/* input/output */
		sgr_t * grid,
		/* input */
		int r,
		int cntposs[MAX_BLOCKS],
		int indposs[MAX_BLOCKS])
{
	int map[grid->w * grid->h];
	/* for each possibility repeated more than once */
	for (int vv = 0; vv < grid->sizes[r]; ++vv) {
		if (cntposs[vv] > 1) {
			/* work out touching possibility intersection */
			bzero(map, sizeof(int) * grid->w * grid->h);
			/* for each block containing given possibility */
			for (int b = 0; b < grid->sizes[r]; ++b) {
				int i = grid->regions[r][b].i;
				int j = grid->regions[r][b].j;
				map[i*grid->w+j] = -1; /* within region */
				int v = grid->values[i*grid->w+j];
				if (v < 0
				 && ~v & (1 << vv)) {
					/* for each adjacent block */
					for (int ii = i-1; ii <= i+1; ++ii)
					for (int jj = j-1; jj <= j+1; ++jj) {
						/* if in map and not in region */
						if (ii < 0 || ii >= grid->h
						 || jj < 0 || jj >= grid->w
						 || map[ii*grid->w + jj] == -1)
							continue;
						/* increment number of times this
						 * adjacent block was touched */
						map[ii*grid->w + jj]++;
					}
				}
			}
			/* for every block in map */
			for (int i = 0; i < grid->h; ++i)
			for (int j = 0; j < grid->w; ++j)
				/* if is touched by every inner block
				 * with given possibility and
				 * is not a final value */
				if (map[i*grid->w + j] == cntposs[vv]
				 && grid->values[i*grid->w + j] < 0) {
					/* remove possibility */
					grid->values[i*grid->w + j] |= 1 << vv;
				}
		}
	}
}

static void sgr_find_subgroups(
		/* input/output */
		sgr_t * grid,
		/* input */
		int r)
{
	//printf("subgroups r=%d\n", r);
	int size = grid->sizes[r];
	char done[MAX_BLOCKS];
	char curr[MAX_BLOCKS];
	bzero(done, MAX_BLOCKS);
	int blkcntposs[MAX_BLOCKS];
	bzero(blkcntposs, sizeof(int) * MAX_BLOCKS);
	for (int b = 0; b < size; ++b) {
		int i = grid->regions[r][b].i;
		int j = grid->regions[r][b].j;
		int v = grid->values[i * grid->w + j];
		if (v > 0)
			continue;
		//putchar('\t');
		for (int vv = 0; vv < size; ++vv) {
			if (~v & (1 << vv)) {
				//printf("%d ", vv+1);
				++blkcntposs[b];
			}
		}
		//printf("\n\tblock #%d nb possibilities: %d\n", b, blkcntposs[b]);
	}
	for (int b = 0; b < size; ++b) {
		int i = grid->regions[r][b].i;
		int j = grid->regions[r][b].j;
		int v = grid->values[i * grid->w + j];
		if (v > 0)
			continue;
		bzero(curr, MAX_BLOCKS);
		int numequal = 1;
		done[b] = 1;
		curr[b] = 1;
		for (int bb = 0; bb < size; ++bb) {
			if (done[bb])
				continue;
			int ii = grid->regions[r][bb].i;
			int jj = grid->regions[r][bb].j;
			int vv = grid->values[ii * grid->w + jj];
			if (vv > 0)
				continue;
			if (v != vv)
				continue;
			++numequal;
			done[bb] = 1;
			curr[bb] = 1;
		}
		if (numequal != blkcntposs[b])
			continue;
		//puts("found subgroup!");
		/* Found subgroup! */
		/* Remove possibilities from blocks not in subgroup */
		for (int vv = 0; vv < size; ++vv) {
			if (!(~v & (1 << vv)))
				continue;
			for (int bb = 0; bb < size; ++bb) {
				if (curr[bb])
					continue;
				int ii = grid->regions[r][bb].i;
				int jj = grid->regions[r][bb].j;
				int vvv = grid->values[ii * grid->w + jj];
				if (vvv > 0)
					continue;
				grid->values[ii * grid->w + jj] |= 1 << vv;
			}
		}
	}
}

void sgr_passregions(sgr_t * grid)
{
	int cntposs[MAX_BLOCKS];
	int indposs[MAX_BLOCKS];
	/* for each region */
	for (int r = 0; r < grid->numregions; ++r) {
		sgr_remove_region_possibilities(grid, r);
		sgr_enumerate_possibilities(grid, r, cntposs, indposs);
		sgr_find_lonely_possibilities(grid, r, cntposs, indposs);
		sgr_find_intersections(grid, r, cntposs, indposs);
		sgr_find_subgroups(grid, r);
	}
}
