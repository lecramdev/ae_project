import osmapi
import re
import os

min_lat = 52.28190
max_lat = 52.28697
min_lon = 8.02050
max_lon = 8.02539

#osna 52.228900, 7.933464; 52.319622, 8.144213
#min_lat = 52.228900
#max_lat = 52.319622
#min_lon = 7.933464
#max_lon = 8.144213

#lk osna 52.043981, 7.651926


scale = (max_lat-min_lat + max_lon-min_lon) / 2 * 1000000 / 100

osm = osmapi.OsmApi()
entries = osm.Map(min_lon, min_lat, max_lon, max_lat)

x = list()
y = list()
l = list()
h = list()
t = list()

for entry in entries:
    if "building" in entry["data"]["tag"] and "name" in entry["data"]["tag"] and "nd" in entry["data"]:
        name = re.sub("\\s+", "_", entry["data"]["tag"]["name"])

        nodes = osm.NodesGet(entry["data"]["nd"])
        avg_lat = 0
        avg_lon = 0
        for node in nodes.values():
            avg_lat += node["lat"]
            avg_lon += node["lon"]
        avg_lat /= len(nodes)
        avg_lon /= len(nodes)
        avg_lat *= 1000000
        avg_lon *= 1000000

        x.append(avg_lon)
        y.append(avg_lat)
        l.append(len(name)*scale)
        h.append(scale)
        t.append(name)

print(len(x))
for i in range(len(x)):
    print("{:.0f} {:.0f} {:.0f} {:.0f} {}".format(x[i], y[i], l[i], h[i], t[i]))
