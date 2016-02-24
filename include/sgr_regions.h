#ifndef __SGR_REGIONS_H__
#define __SGR_REGIONS_H__

#include <sgr.h>
#include <stdlib.h>

void sgr_remove_region_possibilities(sgr_t * grid, int r);

/* In a given region, count the occurrences of each
 * possibility, and the last index of each. */
void sgr_enumerate_possibilities(
        /* input */
        sgr_t * grid, int r,
        /* output */
        char cntposs[5],
        char indposs[5]);

/* Uses cntposs and indposs, generated from
 * sgr_enumerate_possibilities. */
void sgr_find_lonely_possibilities(
        /* input/output */
        sgr_t * grid,
        /* input */
        int r,
        char cntposs[5],
        char indposs[5]);

/* For a given region, look for blocks (outside of it)
 * that touch every block within the region containing
 * a possibility, then remove that possibility from
 * the external block.
 * Uses cntposs and indposs, generated from
 * sgr_enumerate_possibilities. */
void sgr_find_intersections(
        /* input/output */
        sgr_t * grid,
        /* input */
        int r,
        char cntposs[5],
        char indposs[5]);

void sgr_passregions(sgr_t * grid);

#endif /* __SGR_REGIONS_H__ */

