# [Suguru](krazydad.com/suguru) Solver

by trosh

## building

running `make` inside the main directory should build
a `suguru` executable.

## usage

    ./suguru FILE.sgr

for example, `./suguru grids/s_v1_b16_3.sgr`

## notes

currently works with pretty simple grids.

the process is relatively simple:
the program parses loads the grid from a `.sgr` file,
then iterates through {values, regions} passes.

*values passes* go through each block in row major order,
affects neighbour cells' possibilities,
then do a second pass to find cells with only one
possibility left.

*regions passes* go through each regions to remove
possibilities according to completed cells within the region,
search for possibilities that appear in only one cell,
then search for single cells outside of region that touch
every cell within region for each possibility.

## todo

- comment comment comment
- split functions way more
- split into files
- split regions when n possibilities are placed in the same n cells
- encapsulate iterator
- determine blocked iterations (not just winning iterations)
- do guesswork :-)
  - do 1-level guesswork
  - do a tree-based guesswork :-O (bruteforce) !!
- display grid with a row major pass
  (store coords -> region array during init?)
- add user options
- add screenshot(s) to this file

