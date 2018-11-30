#ifndef __VIRTIO_NET_H__
#define __VIRTIO_NET_H__


#define VIRTIO_IO_NET_MAC_ADDR VIRTIO_IO_DEVICE_SPECIFIC
#define VIRTIO_NET_RECV_QUEUE  0
#define VIRTIO_NET_TRANS_QUEUE 1
#define VIRTIO_NET_CTRL_QUEUE  2
#define VIRTIO_NET_F_MAC       (1 << 5)
#define VIRTIO_NET_F_STATUS    (1 << 16)

struct virtio_net_request_header {
  uint8_t   gso_type;
  uint8_t   flags;
  uint16_t  hdr_len;
  uint16_t  gso_size;
  uint16_t  csum_start;
  uint16_t  csum_offset;
} PACKED;

extern channel_t virtio_net_listener;

void virtio_net_init(void);
result_t virtio_net_transmit(void *data, size_t size);
result_t virtio_net_receive(void **data, size_t *size);
void virtio_net_get_hwaddr(uint8_t *hwaddr);

#endif
