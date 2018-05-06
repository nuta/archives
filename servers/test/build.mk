name := test
objs := main.o
libs :=
requires := logging exit discovery benchmark

include servers/server.mk
