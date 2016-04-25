#ifndef __LIBPCI_PCI_H__
#define __LIBPCI_PCI_H__

struct libpci_config_header {
    uint16_t  device;
    uint16_t  vendor;
    uint16_t  status;
    uint16_t  command;

    uint8_t   class_code;
    uint8_t   subclass_code;
    uint8_t   interface;
    uint8_t   revision;

    uint8_t   bist;
    uint8_t   type;
    uint8_t   timer1;
    uint8_t   cache_line_size;

    uint32_t  bar0;
    uint32_t  bar1;

    uint8_t   timer2;
    uint8_t   sub_bus;
    uint8_t   bus2;
    uint8_t   bus;

    uint16_t  status2;
    uint8_t   io_limit;
    uint8_t   io_base;

    uint16_t  memory_limit;
    uint16_t  memory_base;

    uint16_t  prefetchable_memory_limit;
    uint16_t  prefetchable_memory_base;
    uint32_t  prefetchable_base1;
    uint32_t  prefetchable_limit1;

    uint16_t  io_limit1;
    uint16_t  io_base1;
    uint32_t  reserved;
    uint32_t  rom_base;
    uint16_t  bridge;
    uint8_t   interrupt_pin;
    uint8_t   interrupt_line;
} PACKED;

#endif
