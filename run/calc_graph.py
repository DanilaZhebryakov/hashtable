import matplotlib.pyplot as plt
import numpy as np
import glob
import math
import parse

def calcBadness(m):
    return sum([m[i]**2 for i in range(len(m))])/sum(m)

data_lines = open("pop_data.txt", "r").readlines()
data = list(map(int,data_lines[5].split('\t')[1:-1]))
print(sum(data))

hist_fig = plt.figure()
hist_plt = hist_fig.add_subplot()
hist_plt.set_ylim(ymax = 30)
hist_plt.set_xlim(xmin = 0)
hist_plt.set_xlim(xmax = 613)
hist_plt.bar([i for i in range(len(data))], data, 1)
hist_fig.savefig("pop_gnu.png")

print([calcBadness(list(map(int,data_lines[i].split('\t')[1:-1]))) for i in range(len(data_lines))])