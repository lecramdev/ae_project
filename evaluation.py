#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import re
import sys

lineparser = re.compile("File: (\\S+) Labeled: (\\d+)/(\\d+) Time: (\\d+.\\d+)s")

files = list()
labeled1 = list()
time1 = list()

with open(sys.argv[1], 'r') as file:
    for line in file:
        match = lineparser.match(line)
        if match:
            files.append(match.group(1))
            labeled1.append(int(match.group(2)))
            time1.append(float(match.group(4)))

labeled2 = list()
time2 = list()
with open(sys.argv[2], 'r') as file:
    for line in file:
        match = lineparser.match(line)
        if match:
            #files.append(match.group(1))
            labeled2.append(int(match.group(2)))
            time2.append(float(match.group(4)))

y = np.arange(len(files))
fig, (axlabled, axtime) = plt.subplots(1, 2)

axlabled.barh(y - 0.2, labeled1, 0.4, label=sys.argv[1])
axtime.barh(y - 0.2, time1, 0.4, label=sys.argv[1])

axlabled.barh(y + 0.2, labeled2, 0.4, label=sys.argv[2])
axtime.barh(y + 0.2, time2, 0.4, label=sys.argv[2])

axlabled.set_xlabel('Labeled')
axlabled.set_yticks(y)
axlabled.set_yticklabels(files)
axlabled.legend()

axtime.set_xlabel('Time')
axtime.set_yticks(y)
axtime.set_yticklabels(files)
axtime.legend()

plt.show()