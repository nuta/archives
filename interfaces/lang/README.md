# lang
An interface implemented by langauge runtime libraries.

## Requirements
### Global config
Append a map into `LANG`. Refer `cpp`'s `package.yml`.

**Example**
```
global_config:
  LANG:
    append:
      c:
        ext:  c
        abbrev: CC
        stub:
          prefix: resea/
          suffix: .h
        compile: '$(CC) $(CFLAGS) -c -o'
        scaffold: ./packages/cpp/scaffold
        genstub:  ./packages/cpp/genstub 
```

