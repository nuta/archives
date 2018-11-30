// Supress compiler warning of libraries.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <stdlib.h>
#include <math.h>

#include <v8.h>
#include <v8/src/base/platform/time.h>
#include <v8/src/base/platform/platform.h>
#include <v8/src/libsampler/v8-sampler.h>
#include <v8/src/base/platform/condition-variable.h>
#include <efijs/message.h>

#pragma GCC diagnostic pop

namespace v8 {

namespace base {

ConditionVariable::ConditionVariable() {}
ConditionVariable::~ConditionVariable() {}

void ConditionVariable::NotifyOne() {
  EFIJS_NYI();
}

void ConditionVariable::NotifyAll() {
  EFIJS_NYI();
}

void ConditionVariable::Wait(Mutex* mutex) {
  EFIJS_NYI();
}

bool ConditionVariable::WaitFor(Mutex* mutex, const TimeDelta& rel_time) {
  EFIJS_NYI();
  return false;
}

Semaphore::Semaphore(int count) {
  native_handle_ = count;
}

Semaphore::~Semaphore() {
}

void Semaphore::Signal() {
  __sync_fetch_and_add(&native_handle_, 1);
}

void Semaphore::Wait() {
  int count = __sync_sub_and_fetch(&native_handle_, 1);
  if (!count)
    EFIJS_PANIC("semaphore wait");
}

bool Semaphore::WaitFor(const TimeDelta& rel_time) {
  EFIJS_NYI();
  return false;
}

}  // namespace base

namespace sampler {

Sampler::Sampler(Isolate* isolate)
  : is_counting_samples_(false),
    js_sample_count_(0),
    external_sample_count_(0),
    isolate_(isolate),
    profiling_(false),
    has_processing_thread_(false),
    active_(false),
    registered_(false) {}

Sampler::~Sampler() {}
void Sampler::Start() { }
void Sampler::Stop() { }
void Sampler::IncreaseProfilingDepth() { }
void Sampler::DecreaseProfilingDepth() { }
void Sampler::DoSample() { }

void Sampler::SetUp() {}
void Sampler::TearDown() {}

} // namespace sampler

namespace base {

namespace {

bool g_hard_abort = false;
const char* g_gc_fake_mmap = NULL;

}  // namespace

double ceiling(double x) {
  return ceil(x);
}

intptr_t OS::CommitPageSize() {
  return 1024*4;
}


// Get rid of writable permission on code allocations.
void OS::ProtectCode(void* address, const size_t size) {
  EFIJS_NYI();
}


void OS::Initialize(int64_t random_seed, bool hard_abort,
                    const char* const gc_fake_mmap) {

  g_hard_abort = hard_abort;
  g_gc_fake_mmap = gc_fake_mmap;
}


// Create guard pages.
void OS::Guard(void* address, const size_t size) {
  EFIJS_NYI();
}


void* OS::GetRandomMmapAddr() {
  EFIJS_NYI();
  return NULL;
}


/*double OS::nan_value() {
  // NAN from math.h is defined in C99 and not in POSIX.
  return NAN;
}*/


int OS::GetCurrentProcessId() {
  return 1;
}


int OS::GetUserTime(uint32_t* secs,  uint32_t* usecs) {
  *secs = static_cast<uint32_t>(1);
  *usecs = static_cast<uint32_t>(1);
  return 0;
}


double OS::TimeCurrentMillis() {
  EFIJS_NYI();
  return 0;
}


double OS::DaylightSavingsOffset(double time, TimezoneCache*) {
  return 0;
}


int OS::GetLastError() {
  return 0;
}


FILE* OS::FOpen(const char* path, const char* mode) {
  EFIJS_DEBUG("tried to open '%s'", path);
  EFIJS_NYI();
  return nullptr;
}


bool OS::Remove(const char* path) {
  return true;
}


bool OS::isDirectorySeparator(const char ch) {
  return ch == '/';
}


char OS::DirectorySeparator() { return '/'; }


FILE* OS::OpenTemporaryFile() {
  EFIJS_NYI();
  return nullptr;
}


const char* const OS::LogFileOpenMode = "w";


void OS::Print(const char* format, ...) {
  va_list args;
  va_start(args, format);
  VPrint(format, args);
  va_end(args);
}


void OS::VPrint(const char* format, va_list args) {
  vprintf(format, args);
}


void OS::FPrint(FILE* out, const char* format, ...) {
  va_list args;
  va_start(args, format);
  VFPrint(out, format, args);
  va_end(args);
}


void OS::VFPrint(FILE* out, const char* format, va_list args) {
  vfprintf(out, format, args);
}


void OS::PrintError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  VPrintError(format, args);
  va_end(args);
}


void OS::VPrintError(const char* format, va_list args) {
  vfprintf(stderr, format, args);
}


int OS::SNPrintF(char* str, int length, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int result = VSNPrintF(str, length, format, args);
  va_end(args);
  return result;
}


int OS::VSNPrintF(char* str,
                  int length,
                  const char* format,
                  va_list args) {
  int n = vsnprintf(str, length, format, args);
  if (n < 0 || n >= length) {
    // If the length is zero, the assignment fails.
    if (length > 0)
      str[length - 1] = '\0';
    return -1;
  } else {
    return n;
  }
}


char* OS::StrChr(char* str, int c) {
  return strchr(str, c);
}


void OS::StrNCpy(char* dest, int length, const char* src, size_t n) {
  strncpy(dest, src, n);
}


std::vector<OS::SharedLibraryAddress> OS::GetSharedLibraryAddresses() {
  return std::vector<OS::SharedLibraryAddress>();
}


int OS::ActivationFrameAlignment() {
  return 16;
}

class TimezoneCache {};


TimezoneCache* OS::CreateTimezoneCache() {
  return NULL;
}


void OS::DisposeTimezoneCache(TimezoneCache* cache) {
  EFIJS_NYI();
}


void OS::ClearTimezoneCache(TimezoneCache* cache) {
  EFIJS_IGNORED_NYI();
}


const char* OS::LocalTimezone(double time, TimezoneCache* cache) {
  // TODO: implement timezones
  return "GMT";
}

double OS::LocalTimeOffset(TimezoneCache* cache) {
  // TODO: implement timezones
  return 0;
}

size_t OS::AllocateAlignment() {
  return 4 * 1024;
}


void* OS::Allocate(const size_t requested,
                   size_t* allocated,
                   bool is_executable) {
  const size_t msize = RoundUp(requested, AllocateAlignment());
  void* mbase = memalign(AllocateAlignment(), msize);
  *allocated = msize;
  return mbase;
}


void OS::Free(void* address, const size_t size) {
  free(address);
}


void OS::Sleep(v8::base::TimeDelta time_delta) {

  EFIJS_NYI();
}


void OS::Abort() {

  EFIJS_NYI();
}


void OS::DebugBreak() {

#if V8_HOST_ARCH_X64
  asm("int $3");
#else
#error Unsupported host architecture.
#endif
}


OS::MemoryMappedFile* OS::MemoryMappedFile::open(const char* name) {
  EFIJS_NYI();
  return nullptr;
}


OS::MemoryMappedFile* OS::MemoryMappedFile::create(const char* name, size_t size,
    void* initial) {

  EFIJS_NYI();
  return nullptr;
}


void OS::SignalCodeMovingGC() {
}


VirtualMemory::VirtualMemory() : address_(NULL), size_(0) { }


VirtualMemory::VirtualMemory(size_t size)
    : address_(ReserveRegion(size)), size_(size) { }


VirtualMemory::VirtualMemory(size_t size, size_t alignment)
    : address_(NULL), size_(0) {

  EFIJS_NYI();
}


VirtualMemory::~VirtualMemory() {
  EFIJS_NYI();
}


bool VirtualMemory::IsReserved() {
  return address_ != NULL;
}


void VirtualMemory::Reset() {
  address_ = NULL;
  size_ = 0;
}


bool VirtualMemory::Commit(void* address, size_t size, bool is_executable) {
  return true;
}


bool VirtualMemory::Uncommit(void* address, size_t size) {
  return true;
}


bool VirtualMemory::Guard(void* address) {
  OS::Guard(address, OS::CommitPageSize());
  return true;
}


void* VirtualMemory::ReserveRegion(size_t size) {
  void* addr = memalign(0x200000, size);
  return addr;
}


bool VirtualMemory::CommitRegion(void* base, size_t size, bool is_executable) {
  return true;
}


bool VirtualMemory::UncommitRegion(void* base, size_t size) {
  return true;
}


bool VirtualMemory::ReleasePartialRegion(void* base, size_t size,
                                         void* free_start, size_t free_size) {
  EFIJS_NYI();
  return true;
}


bool VirtualMemory::ReleaseRegion(void* base, size_t size) {
  free(base);
  return true;
}


bool VirtualMemory::HasLazyCommits() {
  return false;
}


class Thread::PlatformData {
 public:
  PlatformData() {}
};


Thread::Thread(const Options& options)
    : data_(new PlatformData()),
      stack_size_(options.stack_size()),
      start_semaphore_(NULL) {
  set_name(options.name());
}


Thread::~Thread() {
  delete data_;
}


void Thread::set_name(const char* name) {
  strncpy(name_, name, sizeof(name_));
  name_[sizeof(name_) - 1] = '\0';
}


void Thread::Start() {
  EFIJS_IGNORED_NYI();
}


void Thread::Join() {
  EFIJS_NYI();
}


Thread::LocalStorageKey Thread::CreateThreadLocalKey() {
  EFIJS_NYI();
  return static_cast<LocalStorageKey>(0);
}


void Thread::DeleteThreadLocalKey(LocalStorageKey key) {
  EFIJS_NYI();
}


void* Thread::GetThreadLocal(LocalStorageKey key) {
  EFIJS_IGNORED_NYI();
  return nullptr;
}


void Thread::SetThreadLocal(LocalStorageKey key, void* value) {
  EFIJS_IGNORED_NYI();
}


} // namespace v8::base
} // namespace v8
