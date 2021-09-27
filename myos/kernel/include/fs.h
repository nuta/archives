enum FILE_FLAG{
  FILE_UNUSED = 0,
  FILE_USING  = 1
};


struct myosFS_Header{

  u8    signature[8];
  u32   version;
  u32   headerSize;
  u32   metaDataAddr;
  u32   dataAddr;
  u32   fileNum;

}__attribute__ ((packed));


struct myosFS_MetaData{

  u8    flag;
  u8    filename[31];
  u32   address;
  u32   size;

}__attribute__ ((packed));


struct myosFS{

  struct myosFS_Header    *Header;
  struct myosFS_MetaData  *MetaData;
  u64                     data;

};

