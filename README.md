# [Suguru](http://krazydad.com/suguru) Solver

by trosh

Coded in *ugly C*© ♥ ♥

![Screenshot of solved Suguru grid](screenshot.png "Screenshot")

## building

running `make` inside the main directory should build
a `suguru` executable.

## usage

    ./suguru FILE.sgr

for example, `./suguru grids/s_v1_b16_3.sgr`

## notes

the process is relatively simple:
the program loads the grid from a `.sgr` file,
then iterates through {values, regions} passes.

- *values passes* go through each cell in row major order,
affects neighbour cells' possibilities,
then do a second pass to find cells with only one
possibility left.
- *regions passes* go through each regions to remove
possibilities according to completed cells within the region,
search for possibilities that appear in only one cell,
then search for single cells outside of region that touch
every cell within region for each possibility.

The storage is relatively compressed, since the
same full grid is used to store possibilities and
final values: in each cell,
- if a value is positive, it is worth the final value
- otherwise its 5 lowest bits describe possibilities.

## todo

- encapsulate iterator
- add user options
