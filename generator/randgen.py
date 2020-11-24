import os
import random

for size in [20, 100, 1000]:#, 10000, 100000, 1000000, 10000000]:
    for density in [0.01, 0.1, 0.2, 0.5, 0.75]:
        for num in range(20):
            with open("{:0>4}_{:0>2.0f}_{:0>2}.txt".format(size, density*100, num+1), "w") as f:
                num = int((size*2)**2 * density)
                f.write("{}\n".format(num))
                for i in range(num):
                    x = random.randint(-size, size)
                    y = random.randint(-size, size)
                    l = random.randint(1, size * 0.25)
                    h = random.randint(1, size * 0.25)
                    t = "Point_{}".format(i)
                    f.write("{:.0f} {:.0f} {:.0f} {:.0f} {} 0 0 0\n".format(x, y, l, h, t))
