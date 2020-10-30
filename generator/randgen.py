import os
import random

for size in [20, 100, 1000]:#, 10000, 100000, 1000000, 10000000]:
    for density in [0.01, 0.1, 0.2, 0.5, 0.75]:
        with open("{}_{:.0f}_.txt".format(size, density*100), "w") as f:
            num = int((size*2+1)**2 * density)
            f.write("{}\n".format(num))
            for i in range(num):
                x = random.randint(-size, size)
                y = random.randint(-size, size)
                l = random.randint(1, size * 0.25)
                h = random.randint(1, size * 0.25)
                t = "Point_{}".format(i)
                f.write("{:.0f} {:.0f} {:.0f} {:.0f} {}\n".format(x, y, l, h, t))
