#include <sgr_solve.h>

#define GUESS_DEPTH 5

void sgr_dup(sgr_t * dst, sgr_t * src) {
	if (dst->values == NULL)
		dst->values = malloc(
			src->w * src->h * sizeof(int));
	if (dst->regionsref == NULL)
		dst->regionsref = malloc(
			src->w * src->h * sizeof(struct regionref));
	for (int n = 0; n < src->w * src->h; ++n)
		dst->values[n] = src->values[n];
	for (int r = 0; r < src->numregions; ++r)
	for (int b = 0; b < src->sizes[r]; ++b)
		dst->regions[r][b] = src->regions[r][b];
	for (int n = 0; n < src->w * src->h; ++n)
		dst->regionsref[n] = src->regionsref[n];
	for (int r = 0; r < src->numregions; ++r)
		dst->sizes[r] = src->sizes[r];
	dst->numregions = src->numregions;
	dst->w = src->w;
	dst->h = src->h;
}

int sgr_checkerror(sgr_t * grid) {
	for (int i = 0; i < grid->h; ++i)
	for (int j = 0; j < grid->w; ++j) {
		int vv = grid->values[i * grid->w + j];
		if (vv == 0) {
			sgr_display(grid);
			fprintf(stderr, "no possibility @ (%d,%d)\n",
				i, j);
			return 1;
		} else if (vv > 0) {
			for (int ii = i-1; ii <= i+1; ++ii)
			for (int jj = j-1; jj <= j+1; ++jj) {
				int ww = grid->values[ii * grid->w + jj];
				if (ii >= 0 && ii < grid->h
				 && jj >= 0 && jj < grid->w
				 && !(ii == i && jj == j)
				 && ww == vv) {
					sgr_display(grid);
					fprintf(stderr, "collision of %d at (%d,%d) and (%d,%d)\n",
						vv, i, j, ii, jj);
					return 1;
				}
			}
		}
	}
	for (int r = 0; r < grid->numregions; ++r)
	for (int t = 1; t <= grid->sizes[r]; ++t) {
		int seen = 0;
		for (int b = 0; b < grid->sizes[r]; ++b) {
			struct coords pos = grid->regions[r][b];
			int v = grid->values[pos.i * grid->w + pos.j];
			if ((v > 0 && v == t)
			 || (v < 0 && ~v & (1 << t-1))) {
				seen = 1;
				break;
			}
		}
		if (!seen) {
			sgr_display(grid);
			fprintf(stderr, "can't find %d in region %d\n",
				t, r+1);
			return 1;
		}
	}
	return 0;
}

char sgr_solve(sgr_t * grid) {
	int ret = -1;
	sgr_t * checkpoint;
	checkpoint = malloc(GUESS_DEPTH * sizeof(sgr_t));
	bzero(checkpoint, GUESS_DEPTH * sizeof(sgr_t));
	struct coords pos[GUESS_DEPTH];
	int guess[GUESS_DEPTH];
	int g = -1;
	while (1) {
		do {
next_guess:
			sgr_passvalues(grid);
			sgr_passregions(grid);
			if (sgr_checkwin(grid)) {
				ret = 1;
				goto exit;
			}
			if (sgr_checkerror(grid)) {
				if (g >= 0) {
					/* Remove guess from checkpoint possibilities */
					checkpoint[g].values[pos[g].i * grid->w + pos[g].j] |= 1 << guess[g] - 1;
					printf("undo %d at (%d,%d)\n",
					       guess[g], pos[g].i, pos[g].j);
					/* Rollback to updated checkpoint */
					sgr_dup(grid, &checkpoint[g]);
					/* Guess stack pop */
					--g;
					goto next_guess;
				}
				fprintf(stderr,
					"Unguessed grid contains an error\n");
				ret = 0;
				goto exit;
			}
		} while (sgr_checkevol(grid));
		/* Start new guess */
		++g;
		/* Find first undecided position */
		for (pos[g].i = 0; pos[g].i < grid->h; ++pos[g].i)
		for (pos[g].j = 0; pos[g].j < grid->w; ++pos[g].j)
			if (grid->values[pos[g].i * grid->w + pos[g].j] < 0)
				goto found_pos;
		fprintf(stderr, "could not find undecided position\n");
		ret = 0;
		goto exit;
found_pos:;
		/* Find first potential guess */
		int v = grid->values[pos[g].i * grid->w + pos[g].j];
		for (guess[g] = 1; guess[g] <= MAX_BLOCKS; ++guess[g])
			if (~v & (1 << guess[g]-1))
				goto found_guess;
		fprintf(stderr, "logic error while guessing at (%d,%d)\n",
			pos[g].i, pos[g].j);
		ret = 0;
		goto exit;
found_guess:
		sgr_dup(&checkpoint[g], grid);
		/* Apply guess! */
		printf("guess %d at (%d,%d)\n",
		       guess[g], pos[g].i, pos[g].j);
		grid->values[pos[g].i * grid->w + pos[g].j] = guess[g];
	}
exit:;
	for (int g = 0; g < GUESS_DEPTH; ++g) {
		if (checkpoint[g].values != NULL)
			free(checkpoint[g].values);
		if (checkpoint[g].regionsref != NULL)
			free(checkpoint[g].regionsref);
	}
	free(checkpoint);
	return ret;
}
