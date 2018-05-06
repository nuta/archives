name := benchmark-server
objs := main.o
libs :=
requires := discovery benchmark

include servers/server.mk
