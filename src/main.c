#include <stdio.h> /* fputs, puts */
#include <sgr.h> /* grid_from_file, display, grid_finalize */
#include <sgr_solve.h> /* solve */

int main(int argc, char * argv[])
{
	sgr_t * grid;
	if (argc != 2) {
		fputs("usage: suguru FILE.sgr", stderr);
		return 1;
	}
	grid = sgr_grid_from_file(argv[1]);
	sgr_display(grid);
	if (sgr_solve(grid))
		puts("\033[32mWIN\033[m");
	else
		puts("\033[31mFAIL\033[m");
	sgr_display(grid);
	sgr_grid_finalize(grid);
	return 0;
}
