#!/bin/bash
#SBATCH --time=30
mpirun --map-by ppr:1:node bin/life rand 100 10000 10000
#mpirun --map-by ppr:1:node valgrind --tool=callgrind bin/life rand 1000 1000 1000
