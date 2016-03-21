#include <resea.h>
void virtio_storage_device_read(channel_t __ch, offset_t a0, size_t a1);
void virtio_storage_device_write(channel_t __ch, offset_t a0, void * a1);
void virtio_net_device_receive(channel_t __ch);
void virtio_net_device_transmit(channel_t __ch, void * a0);
void virtio_net_device_get_hardware_address(channel_t __ch);
