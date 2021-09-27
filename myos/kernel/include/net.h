extern struct NetInfo Net;


typedef u8 MACAddress[6];
typedef unsigned short int  Port;
typedef u32  IPAddress;
typedef u32  SubnetMask;


/*==================================
*  Frame
*=================================*/

#define FRAME_HEADER_LENGTH  18
#define FRAME_DATA_MAX       1000

enum FRAME_TYPE{
  FRAME_IP  = 0x0800,
  FRAME_ARP = 0x0806
};


struct MacFrame{
  MACAddress     hwDest;
  MACAddress     hwSrc;
  u16            type;
  u8             data[FRAME_DATA_MAX];
}__attribute__ ((packed));




/*==================================
*  ARP
*=================================*/


#define  ARP_PACKET_SIZE    46
#define  ARP_CACHE_MAX      20


enum ARPCACHE_STATUS{
  ARPCACHE_WAIT_REPLY,
  ARPCACHE_CACHED
};

struct ARPCacheEntry{
  IPAddress   IPAddr;
  MACAddress  MACAddr;
  enum ARPCACHE_STATUS status;
};


struct ARPPacket{
  u16                hardwareType;
  u16                protocolType;
  u8                 hardwareSize;
  u8                 protocolSize;
  u16                operation;
  MACAddress         hwSrc;
  IPAddress          IPSrc;
  MACAddress         hwDest;
  IPAddress          IPDest;
}__attribute__ ((packed));



/*==================================
*  UDP
*=================================*/

#define  UDP_HEADER_LENGTH 8


struct UDPPacket{
  u16 portSrc;
  u16 portDest;
  u16 dataLength;
  u16 checksum;
  u8  data[1000];
}__attribute__ ((packed));




/*==================================
*  TCP
*=================================*/


#define  TCP_DEFAULT_WINDOW_SIZE  400
#define  TCP_DATASIZE_MAX         1300
#define  TCP_HEADER_LENGTH        5   /* (!) 単位は4バイト */
#define  TCP_CONT_MAX             30

enum TCP_CTRL_FLAG{
  TCP_CTRLFLAG_FIN  = 0x01,
  TCP_CTRLFLAG_SYN  = 0x02,
  TCP_CTRLFLAG_RST  = 0x04,
  TCP_CTRLFLAG_ACK  = 0x10
};




struct TCPPacket{
  u16     portSrc;
  u16     portDest;
  u32     seqNum;
  u32     ackNum;
  u16     info;
  u16     windowSize;
  u16     checksum;
  u16     urgPointer;
  u8      data[TCP_DATASIZE_MAX];
}__attribute__((packed));




enum TCP_CONT_STATUS{
  TCP_CONT_CLOSED,      /* 未接続 */
  TCP_CONT_SYN_SENT,    /* コネクション確立要求を送信し、SYN+ACKを待っている状態 */
  TCP_CONT_ESTABLISHED, /*  接続  */
  TCP_CONT_ACK_WAIT,    /* データを送信し、ACKを待っている状態 */
  TCP_CONT_SENDING,     /* データを送信している状態 */
  TCP_CONT_FIN_WAIT1,   /* FINを送信し、ACKを待っている状態 */
  TCP_CONT_FIN_WAIT2,   /* ACKを受信し、FINを待っている状態 */
  TCP_CONT_TIME_WAIT    /* FINを受信し、ACKを送信し、セグメントを捨てている状態 */
};


struct TCPConnection{
  enum TCP_CONT_STATUS   status;
  IPAddress              IPDest;
  Port                   portDest;
  Port                   portSrc;
  u32                    seqNum;
  u32                    ackNum;
  u16                    windowSize;
  u32                    sendP;
  u32                    length;
  u8                    *data;

  struct{
    u32  ackNum;
    u32  seqNum;
  } LatestRecvPacket;
};



/*==================================
*  IP
*=================================*/

#define  IP_BROADCAST_ADDRESS   0xffffffff

#define  IP_PACKET_TTL          0x80
#define  IP_ID_MAX              0xffff
#define  IP_HEADER_LENGTH       20 /* バイト単位 */

enum IP_PROTOCOL_TYPE{
  IP_PROTOCOL_ICMP =1,
  IP_PROTOCOL_TCP  =6,
  IP_PROTOCOL_UDP  =17
};

struct IPPacket{
  u8                       version_headerLength;
  u8                       tos;
  u16                      dataLength;
  u16                      id;
  u16                      flag_fragOffset;
  u8                       ttl;
  u8                       protocol;
  u16                      checksum;
  IPAddress                IPSrc;
  IPAddress                IPDest;
  u8                       data[1000];
}__attribute__ ((packed));



/*==================================
*  DNS
*=================================*/


#define  DNSFLAG_QR_QUERY        0x0
#define  DNSFLAG_OPCODE_STQUERY  00
#define  DNSFLAG_RD_REC          0x0100
#define  DNS_PORT_SRC            12345
#define  DNS_PORT_DEST           53
#define  DNS_IP_DEST             0x0a000203


struct DNSPacket{

  struct{
    u16    id;
    u16    flag;
    u16    qdCount;
    u16    anCount;
    u16    nsCount;
    u16    arCount;
  }__attribute__ ((packed)) Header;

  u8  question[250];
}__attribute__ ((packed));



/*==================================
*  ICMP
*=================================*/


struct EchoRequest{
  u8    type;
  u8    code;
  u16   checksum;
  u16   id;
  u16   sequence;
  u32   data;
}__attribute__ ((packed));




/*==================================
*  DHCP
*=================================*/


#define  DHCP_OPTION_TYPE                    0x35
#define  DHCP_OPTION_ROUTER_IP_ADDR          0x03
#define  DHCP_OPTION_REQUEST_IP_ADDR         0x32
#define  DHCP_OPTION_REQUEST_IP_ADDR_LENGTH  0x04
#define  DHCP_OPTION_DNS_SERVER_IP_ADDR      0x06
#define  DHCP_OPTION_SUBNETMASK              0x01
#define  DHCP_OPTION_PADDING                 0x00
#define  DHCP_OPTION_END                     0xffffffff //?

enum DHCP_PACKET_TYPE{
  DHCP_DISCOVER = 0x01,
  DHCP_OFFER    = 0x02,
  DHCP_REQUEST  = 0x03,
  DHCP_ACK      = 0x05
};


struct DHCPPacket{
  u8           opcode;
  u8           hwType;
  u8           hwAddrLength;
  u8           hopCount;
  u32          id;
  u16          sec;
  u16          unused;
  IPAddress    IPClient;
  IPAddress    IPUser;
  IPAddress    IPServer;
  IPAddress    IPGateway;
  u8           clientHwAddr    [16];
  u8           serverHostName  [64];
  u8           stFilename      [128];

  struct{
    u32          magicCode;
    u8           type       [3];
    u8           clientID   [3];
    MACAddress   hwSrc;
    u8           unused     [48];
  }__attribute__ ((packed)) vendorInfo;
}__attribute__ ((packed));




/*==================================
*  NetInfo
*=================================*/


struct NetInfo{
  IPAddress  IPAddr;
  IPAddress  requestedIPAddr;
  MACAddress MACAddr;
  SubnetMask subnetMask;
  IPAddress  RouterIPAddr;
  IPAddress  DNSServerIPAddr;
  bool       gotIPAddr;

  struct{
    u16 id;
  } IP;

  struct{
    struct ARPCacheEntry Cache[ARP_CACHE_MAX];
  } ARP;

  struct{
    struct TCPConnection Connection[TCP_CONT_MAX];
  } TCP;

};





/*==================================
*  関数
*=================================*/

//sys.c
void sendPacket    (void *frame, u32 size);

//net.c
void sendFrame    (MACAddress hwDest, enum FRAME_TYPE type, void *data, u32 size);
void receiveFrame (void *frame);

//ARP
void sendARPRequest   (IPAddress to);
void receiveARPPacket (struct ARPPacket *Packet);


//UDP
void sendUDPPacket    (IPAddress         IPDest, Port portDest,   Port portSrc, void *data, int size);
void receiveUDPPacket (struct UDPPacket *Packet, IPAddress IPSrc, u16 size);


//IP
void sendIPPacket    (IPAddress IPDest, enum IP_PROTOCOL_TYPE protocol, void *data, unsigned int size);
void receiveIPPacket (struct IPPacket *Packet);


//ICMP
void sendEchoRequest   (IPAddress to);
void receiveICMPPacket (struct EchoRequest *Request, IPAddress IPSrc);


//DNS
IPAddress resolveDomainByDNS (const char *qDomain);


//DHCP
void getIPAddrByDHCP   (void);
void sendDHCPPacket    (enum DHCP_PACKET_TYPE type);
void receiveDHCPPacket (struct DHCPPacket *Packet);


//TCP
void         sendDataByTCP        (uIndex                ct,      void      *data,     size length);
void         sendTCPPacket        (struct TCPConnection *cont,    u8         ctrlBits, int  size, void *data, bool isHeaderOnly);
void         receiveTCPPacket     (struct TCPPacket     *Packet,  IPAddress  IPSrc,    u16  size);
uIndex       createTCPConnection  (IPAddress             IPDest,  Port       portDest, Port portSrc);
unsigned int getTCPContBySrcPort  (Port port);
void         closeTCPConnection   (uIndex i);


//ライブラリ
void        setMACAddress (void *addr1, void *addr2);
MACAddress *getMACAddress (IPAddress IPAddr);

void he2ne (void *data, int size);
void ne2he (void *data, int size);

