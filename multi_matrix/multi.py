#!/bin/python
import sys
import config
a = [[0 for j in range(config.K)] for i in range(config.I)]
b = [[0 for j in range(config.J)] for i in range(config.K)]
for line in sys.stdin.readlines():
    m, i, j, v = map(int, line.split())
    if m == 0:
        a[i][j] = v
    else:
        b[i][j] = v

for i in range(config.I):
    for j in range(config.J):
        c = 0
        for k in range(config.K):
            c += a[i][k] * b[k][j]
        print(i, str(j) + "\t" + str(c))

