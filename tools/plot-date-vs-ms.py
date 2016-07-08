#!/usr/bin/env python3
# pip3 install matplotlib numpy
import matplotlib.pyplot as plt
import matplotlib.dates as md
import numpy as np
import datetime as dt
import time
import sys

data = np.genfromtxt(sys.argv[1], delimiter=',', names=['time', 'date', 'ms', 'log'])
dates = np.vectorize(dt.datetime.fromtimestamp)((data['time']))
plt.plot(dates, data['ms'])

ax=plt.gca()
xfmt = md.DateFormatter('%Y-%m-%d %H:%M:%S')
ax.xaxis_date()
ax.xaxis.set_major_formatter(xfmt)
plt.xticks(rotation=30)

plt.show()
