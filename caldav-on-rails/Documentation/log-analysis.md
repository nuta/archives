Log Analysis
============

## Requirements

```
$ install python3
$ pip3 install numpy matplotlib
```

## Plotting

```
$ ./tools/parse-log production.log|grep HTTP > http.csv
$ ./tools/parse-log production.log|grep SQL  > sql.csv
$ ./tools/plot-date-vs-ms.py http.csv
$ ./tools/plot-date-vs-ms.py sql.csv
```
