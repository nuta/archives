# lang
An interface implemented by langauge runtime libraries.

## Requirements
### Global config
Append a dict into `LANGS`.

**Example (C++)**
```
global_config:
  LANGS:
    append:
      cpp:
        ext:  cpp
        abbrev: CXX
        stub:
          prefix: resea/
          suffix: .h
        compile:  '{{ CXX }} {{ CXXFLAGS }} -c -o'
        mkdeps:   '{{ CPP_DIR }}/mkdeps "{{ CXX }} {{ CXXFLAGS }}"'
        scaffold: '{{ CPP_DIR }}/scaffold'
        genstub:  '{{ CPP_DIR }}/genstub'
        doctor:   '{{ CPP_DIR }}/doctor'
  CXXFLAGS:
    append_words: -Wall -ggdb3 -std=c++14
```
