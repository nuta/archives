#ifndef __KERNEL_DATETIME_H__
#define __KERNEL_DATETIME_H__

#include <resea.h>

namespace kernel {
namespace datetime {

result_t get_date(uint32_t *year, uint32_t *date, uint32_t *nsec);
void init();

} // namespace datetime
} // namespace kernel

#endif
