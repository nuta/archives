name := benchmark-client
objs := main.o
libs :=
requires := discovery benchmark

include servers/server.mk
