#include <sgr_solve.h>
#include <sgr.h>

char sgr_solve(sgr_t * grid) {
    char evol;
    sgr_display(grid);
    do {
        sgr_passvalues(grid);
        sgr_passregions(grid);
    } while ((evol = sgr_checkevol(grid))
          && !sgr_checkwin(grid));
    return evol;
}

