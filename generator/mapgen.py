import overpy
import re
import os

#campus 52.28190, 8.02050, 52.28697, 8.02539
#osna 52.228900, 7.933464, 52.319622, 8.144213
#lk osna 52.043981, 7.651926, 52.695859, 8.477087
#westniedersachen 51.993725, 6.703748, 53.843298, 9.541409
#niedersachsen 51.485112, 6.861938, 53.637802, 11.588042
#norddeutschland 50.672107, 5.855683, 55.051746, 15.352798
#deutschland (too big) 47.459689, 6.347216, 55.051746, 15.352798

scale = 50

op = overpy.Overpass()
entries = op.query("[out:json];node[amenity][name](51.993725, 6.703748, 53.843298, 9.541409);out body;")

x = list()
y = list()
l = list()
h = list()
t = list()

for node in entries.nodes:
    name = re.sub("\\s+", "_", node.tags.get("name", "n/a"))

    x.append(node.lon * 1000000)
    y.append(node.lat * 1000000)
    l.append(len(name)*scale)
    h.append(scale)
    t.append(name)

print(len(x))
for i in range(len(x)):
    print("{:.0f} {:.0f} {:.0f} {:.0f} {}".format(x[i], y[i], l[i], h[i], t[i]))
