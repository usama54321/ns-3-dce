import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd


outputfile = sys.argv[1]
xlabel = "Time (s)"
ylabel = "Latency (ms)"
title = ""
lTitle = "Node"

ipData0 = []
ipData1 = []
ipData2 = []



for index in range (3):
  path = "../"
  path += str(index)
  with open (path) as f:
    for line in f:
      if (index == 0):
        ipData2.append (int (line.rstrip ('\n')))
      elif (index == 1):
        ipData1.append (int (line.rstrip ('\n')))
      elif (index == 2):
        ipData0.append (float (line.rstrip ('\n')))

time = []
t = 0.0
for i in range (len(ipData0)-1):
  t = t + 0.2
  time.append (t)

ipData0.pop ()
print len (ipData0)
print len (ipData1)
print len (ipData2)


'''
Set labels and titles
'''
plt.xlabel (xlabel)
plt.ylabel (ylabel)
#plt.title (title)

#ax = plt.subplot(111)

'''
Plot graph against 0th 
column to every column.
'''

plt.plot (time, ipData0, label="node-1 to node-2");
plt.plot (time, ipData1, label="node-1 to node-4");
plt.plot (time, ipData2, label="node-1 to node-5");


'''
Set legends and title 
'''
#box = ax.get_position()
#ax.set_position([box.x0, box.y0, box.width * 0.80, box.height])
plt.legend(loc='center right')

'''
Save the graph
'''
plt.savefig (outputfile)


