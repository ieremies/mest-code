#!/usr/bin/env python3

import os

for filename in os.listdir("./inst/color3/"):
    if filename.endswith("b") or filename.startswith("."):
        continue
    nodes = -1
    output = []
    degrees = []

    with open("./inst/color3/" + filename, "r") as file:
        print(filename, end="\t")
        for line in file.readlines():
            line = line.strip()
            if len(line) <= 0:
                continue
            if line[0] == "p":
                # this is beginning line
                nodes = int(line.split(" ")[2])
                output = [-1] * nodes
                degrees = [0] * nodes
            if line[0] == "e":
                try:
                    index_1 = int(line.split()[1]) - 1
                    index_2 = int(line.split()[2]) - 1
                except:
                    print(line)
                degrees[index_1] += 1
                degrees[index_2] += 1
        print(max(degrees))
