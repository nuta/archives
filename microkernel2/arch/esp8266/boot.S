.section .boot
.globl esp8266_boot
esp8266_boot:
    j 1f
.literal_position
.align 4
1:
    movi   sp, __stack
    movi   a4, esp8266_init
    callx0 a4
    ret.n

