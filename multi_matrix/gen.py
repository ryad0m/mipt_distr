#!/bin/python
import random
import config
ni, nk, nj = config.I, config.K, config.J

def print_matrix(a, n, m):
    for i in range(n):
        for j in range(m):
            print(a, i, j, random.randint(-1000, 1000))


print_matrix(0, ni, nk)
print_matrix(1, nk, nj)

