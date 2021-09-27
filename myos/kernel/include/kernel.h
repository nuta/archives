//
//  ウェルカムメッセージ
//

#define WELCOME_MSG  "\nWelcome to myos 3.0.\n\n"




//
// 型定義
//

typedef unsigned char  bool;
#define  true      1
#define  false     0

#define  NULL  ((void *) 0)

typedef signed char                s8;
typedef signed short int           s16;
typedef signed int                 s32;
typedef signed long long int       s64;

typedef unsigned char              u8;
typedef unsigned short int         u16;
typedef unsigned int               u32;
typedef unsigned long long int     u64;

typedef u64     uCount;
typedef u64     uIndex;
typedef u64     address;
typedef char*   str;
typedef u64     size;

#define  UCOUNT_MAX     0xffffffffffffffff


// ページング関連
#define  PML4E_NUM      512
#define  PDPT_SIZE      32768 //32KB
#define  PDT_SIZE       32768
#define  PT_SIZE        32768
#define  PDPTE_SIZE     8
#define  PDTE_SIZE      8
#define  PTE_SIZE       8
#define  PAGE_SIZE      0x1000

enum PG_RW{
  PG_READABLE = 0,
  PG_WRITABLE = 1
};


enum PG_US{
  PG_KERNEL  = 0,
  PG_USER    = 1
};


// カーネルイメージの情報
#define  KERNELIMG_SIZE          0x10000

// 各データのアドレス
#define  BASE_ADDRESS             0xc0000000000
#define  KERNELIMG_ADDRESS        (BASE_ADDRESS+0x8000)
#define  BOOT4_ADDRESS            (BASE_ADDRESS+0x8c00)
#define  PAGING_TABLE_ADDRESS     (BASE_ADDRESS+0x10000)
#define  IDT_ADDRESS              (BASE_ADDRESS+0x100000)
#define  TEMP_ADDRESS             (BASE_ADDRESS+0x120000)
#define  GDT_ADDRESS              (BASE_ADDRESS+0x130000)
#define  TSS_ADDRESS              (BASE_ADDRESS+0x140000)
#define  REGS_SAVE_ADDRESS        (BASE_ADDRESS+0x150000)
#define  IDTR_ADDRESS             TEMP_ADDRESS
#define  GDTR_ADDRESS             TEMP_ADDRESS
#define  KERNEL_ADDRESS_END       (BASE_ADDRESS+0x2000000)

// GDT
#define  GDT_LENGTH                      (16*7-1)
#define  GDT_KERNEL_CODE_SELECTOR        8*1
#define  GDT_KERNEL_DATA_SELECTOR        8*2
#define  GDT_USER_CODE_SELECTOR          8*3
#define  GDT_USER_DATA_SELECTOR          8*4
#define  GDT_TSS_SELECTOR                8*5
#define  GDT_LIMIT2                      0xa0
#define  GDT_KERNEL_CODESEG              0x9a
#define  GDT_KERNEL_DATASEG              0x92
#define  GDT_TSS                         0x89
#define  GDT_TSS_LIMIT                   0x2068
#define  GDT_USER_CODESEG                0xfa
#define  GDT_USER_DATASEG                0xf2

// IDT
#define  IDT_LENGTH               0x7ff
#define  IDT_TIMER_SELECTOR       0x20
#define  IDT_INT_HANDLER          0x8e00

// 表示設定
#define  CUI_FONT_COLOR           0x07

// タイマ
#define  TASK_SWITCH_INTERVAL     20
#define  TIMER_INTERVAL           1000

// 割り込み
#define INTERRUPT_MASK_MASTER     0xfa
#define INTERRUPT_MASK_SLAVE      0xf7

// IO ポート
#define IOPORT_INTMASK_MASTER     0x21
#define IOPORT_INTMASK_SLAVE      0xa1


// キーコード
#define KEY_ESC         0x01
#define KEY_BACKSPACE   0x0e
#define KEY_TAB         0x0f
#define KEY_ENTER       0x1c
#define KEY_HANKAKU     0x29
#define KEY_HIRAGANA    0x70
#define KEY_SPACE       0x39
#define KEY_SYSREQ      0x54
#define KEY_ALT         0x38
#define KEY_CTRL        0x1d
#define KEY_R_SHIFT     0x36
#define KEY_L_SHIFT     0x2a
#define KEY_CAPSLOCK    0x3a
#define KEY_NUMLOCK     0x45
#define KEY_SCROLLLOCK  0x46
#define KEY_F1          0x3b
#define KEY_F2          0x3c
#define KEY_F3          0x3d
#define KEY_F4          0x3e
#define KEY_F5          0x3f
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_F11         0x57
#define KEY_F12         0x58


/* FIXME */
#include <stdarg.h>

// タスク
#define  TASK_ENTRY_MAX     1000
#define  TASK_RFLAGS        0x202
#define  TASK_STACK_SIZE    0x10000
#define  USER_STACK_ADDRESS 0xa0000

// TSS
#define  TSS_RSP0       0xc0002000000
#define  TSS_IST1       0xc0002000000

// Screen
#define  VRAM_ADDRESS    BASE_ADDRESS+0xb8000
#define  SCREEN_X_MAX    80
#define  SCREEN_Y_MAX    25



//
//  構造体
//


struct st_Registers{
  u64  rip;
  u64  cr3;
  u64  rax;
  u64  rbx;
  u64  rcx;
  u64  rdx;
  u64  rdi;
  u64  rsi;
  u64  rbp;
  u64  rsp;
  u64  r8;
  u64  r9;
  u64  r10;
  u64  r11;
  u64  r12;
  u64  r13;
  u64  r14;
  u64  r15;
} __attribute__((packed));


struct st_TSS{
   u32 reserved0;
   u64 rsp0;
   u64 rsp1;
   u64 rsp2;
   u64 reserved1;
   u64 ist[7];
   u64 reserved2;
   u16 reserved3;
   u16 ioMapBase;
   u8  ioPermMap[8192];
} __attribute__((packed));


struct PML4{
  u64 entry [PML4E_NUM];
}__attribute__ ((packed));


struct TSSDescriptor{
 u16   limit1;
 u16   base1;
 u8    base2;
 u8    info;
 u8    limit2;
 u8    base3;
 u32   base4;
 u32   reserved;
}__attribute__ ((packed));


struct SegDescriptor{
 u16   limit1;
 u16   base1;
 u8    base2;
 u8    info;
 u8    limit2;
 u8    base3;
}__attribute__ ((packed));


struct IntDescriptor{
 u16   offset1;
 u16   cs;
 u16   info;
 u16   offset2;
 u32   offset3;
 u32   reserved;
}__attribute__ ((packed));


struct st_GDTR{
  u16 length;
  u64 address;
}__attribute__ ((packed));


struct st_IDTR{
  u16 length;
  u64 address;
}__attribute__ ((packed));


struct ScreenInfo{
  unsigned int  x;
  unsigned int  y;
  unsigned int  xMax;
  unsigned int  yMax;
  u64           VRAMAddress;
};


struct TaskEntry{
  enum {
    TASK_UNUSED,
    TASK_RUNNING
  }status;

  struct st_Registers  Regs;
};

struct TaskInfo{
  struct TaskEntry  Entry[TASK_ENTRY_MAX];
  uIndex            running;
  uIndex            active;
  uCount            runningNum;
};


//
// グローバル変数
//

extern struct ScreenInfo   Screen;
extern struct TaskInfo     Task;




//
//  関数
//

// net.c
void initNetwork (void);


// fs.c
void initFS      (void);
s64  searchFile  (const char *filename);
s64  getFilesize (const char *filename);
bool fileExists  (const char *filename);
bool loadFile    (void *buf, const char *filename, const size bufsize);


// api.c
void initAPI   (void);


// timer.c
void initTimer (void);


// task.c
void startTask  (const char *filename);
void initTask   (void);
void quitTask   (uIndex i);
bool newTask    (u64 rip, u64 cr3, u64 rsp);
void saveTask   (uIndex i, struct st_Registers *Regs);
void switchTask (void);


// halt.c
void halt (void);


//memory,c
bool isActiveMemoryAddress (address addr);
void *kmalloc              (size memsize);


//paging.c
void changePagingTable (struct PML4 *Pgtbl);
void setPagingTable (struct PML4 *Pml4,  address pAddr, address  vAddr,
                     enum PG_RW rw,      enum PG_US us);


// elf.c
address analyzeCode (void *code);

// descriptor.c
void setSegDescriptor (unsigned int dsc, u32 base, u32 limit,  u16 info);
void setIntDescriptor (unsigned int dsc, u16 cs,   u64 offset, u16 info);
void setTSSDescriptor (unsigned int dsc, u64 base, u16 info);

// std.c
int strncmp (const char *str1 , const char *str2 , unsigned int len);



// descriptor.c
void setIntDescriptor (unsigned int dsc, u16 cs, u64 offset, u16 info);



// halt.c
void   halt  (void);

// core/assembly.asm
void asm_APIEntry     (void);
void asm_initSYSCALL  (u64, u16, u16, u64);
void asm_switchTask   (struct st_Registers *);


// lib/assembly.asm
void   asm_hlt     (void);
void   asm_cli     (void);
void   asm_sti     (void);
void   asm_ltr     (u16 ltr);
void   asm_lgdt    (u64 gdtr);
void   asm_lidt    (u64 idtr);
void   asm_out8    (u16 port, u8 data);
u8     asm_in8     (u16 port);
u64    asm_setCR3  (u64 cr3);
u64    asm_loadCR2 (void);
void   asm_cpuid   (u64 num, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);



// print.c
void   print       (const char *format, ...);
void   printchar   (const char ch, const unsigned char color);
int    vprintf     (const char *format, va_list vargs);



// intHandler.asm
void  asm_intTimerHandler     (void);
void  asm_exception0         (void);
void  asm_exception1         (void);
void  asm_exception2         (void);
void  asm_exception3         (void);
void  asm_exception4         (void);
void  asm_exception5         (void);
void  asm_exception6         (void);
void  asm_exception7         (void);
void  asm_exception8         (void);
void  asm_exception9         (void);
void  asm_exception10        (void);
void  asm_exception11        (void);
void  asm_exception12        (void);
void  asm_exception13        (void);
void  asm_exception14        (void);
void  asm_exception15        (void);
void  asm_exception16        (void);
void  asm_exception17        (void);
void  asm_exception18        (void);
void  asm_exception19        (void);

