#include <sgr_solve.h>

char sgr_solve(sgr_t * grid) {
	char evol;
	do {
		sgr_passvalues(grid);
		sgr_passregions(grid);
	} while ((evol = sgr_checkevol(grid))
	       && !sgr_checkwin(grid));
	return evol;
}
