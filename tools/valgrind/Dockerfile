FROM ubuntu:18.04
RUN apt-get update -qy && apt-get install -qy valgrind clang build-essential python3 git
CMD sh -c "cd /ena && make clean && make -j2 && valgrind --leak-check=full ./ena test/if.ena"
