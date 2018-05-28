import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd

filename = sys.argv[1]

outputfile = sys.argv[2]
xlabel = "time(s)"
ylabel = "latency(ms)"
title = ""
lTitle = "Node"

ips = ["10.1.2.1", "10.2.1.1", "10.2.2.1"]
ipData0 = []
ipData1 = []
ipData2 = []

def returnListIndex (ip):
  k = -1;
  for i in ips:
    k = k + 1
    if (ip == i):
      return k;
  return k;

with open (filename) as f:
  for line in f:
    lineSplit = line.split ()
    index = returnListIndex (lineSplit[1])
    #print index
    #print lineSplit[1]+" "+lineSplit[6]
    if (index == 0):
      ipData0.append (lineSplit[6])
    elif (index == 1):
      ipData1.append (lineSplit[6])
    elif (index == 2):
      ipData2.append (lineSplit[6])

ipData0.pop ()
time = []
t = 0.0
for i in range (len(ipData0)):
  t = t + 0.2
  time.append (t)

print len (ipData0)
print len (ipData1)
print len (ipData2)

'''
Set labels and titles
'''
plt.xlabel (xlabel)
plt.ylabel (ylabel)
#plt.title (title)

ax = plt.subplot(111)

'''
Plot graph against 0th 
column to every column.
'''

ax.plot (time, ipData0);
ax.plot (time, ipData1);
ax.plot (time, ipData2);

legends = ["l1", "r1", "r2"]

'''
Set legends and title 
'''
box = ax.get_position()
ax.set_position([box.x0, box.y0, box.width * 0.80, box.height])
ax.legend(legends, loc='center left', title=lTitle, bbox_to_anchor=(1, 0.5))

'''
Save the graph
'''
plt.savefig (outputfile)


