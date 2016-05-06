#include <resea.h>
#include <resea/datetime.h>
#include "kernel.h"


/** handles datetime.get_date */
void kernel_datetime_get_date(channel_t __ch) {
    result_t r;
    uint32_t year, date, nsec;

    r = kernel_get_date(&year, &date, &nsec);
    send_datetime_get_date_reply(__ch, r, year, date, nsec);
}
