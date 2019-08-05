#ifndef __SGR_VALUES_H__
#define __SGR_VALUES_H__

#include <sgr.h>

void sgr_remove_touching_possibilities(sgr_t * grid);

void sgr_find_final_values(sgr_t * grid);

void sgr_passvalues(sgr_t * grid);

/* get value in region r at cell b of grid */
int sgr_get_rb(sgr_t * grid, int r, int b);

/* set value in region r at cell b of grid */
void sgr_set_rb(sgr_t * grid, int r, int b, int v);

#endif /* __SGR_VALUES_H__ */
