#include <sgr_values.h>

void sgr_remove_touching_possibilities(sgr_t * grid) {
	/* for each cell */
	for (int i = 0; i<grid->h; i++)
	for (int j = 0; j<grid->w; j++) {
		int v = grid->values[i*grid->w+j];
		/* if is final value */
		if (v > 0) {
			/* for each adjacent cell
			 * (including current one, but it is final so it's ok) */
			/* for each adjacent block */
			for (int ii = i-1; ii <= i+1; ii++)
			for (int jj = j-1; jj <= j+1; jj++) {
				/* if in grid */
				if (ii < 0 || ii >= grid->h
				 || jj < 0 || jj >= grid->w)
					continue;
				int vv = grid->values[ii*grid->w+jj];
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
	/* for each cell */
	for (int i = 0; i<grid->h; i++)
	for (int j = 0; j<grid->w; j++) {
		int v = grid->values[i*grid->w+j];
		/* if not final value */
		if (v < 0) {
			/* if contains only one possibility */
			for (int vv = 0; vv < MAX_BLOCKS; ++vv) {
				if (v == -(1 << vv) - 1)
					v = vv + 1;
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

int sgr_get_rb(sgr_t * grid, int r, int b) {
	int i = grid->regions[r][b].i;
	int j = grid->regions[r][b].j;
	return grid->values[i*grid->w+j];
}

void sgr_set_rb(sgr_t * grid, int r, int b, int v) {
	int i = grid->regions[r][b].i;
	int j = grid->regions[r][b].j;
	grid->values[i*grid->w+j] = v;
}
