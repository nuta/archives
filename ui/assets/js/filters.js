import format from 'date-fns/format'

export function date(unixtime, fmt) {
  return format(new Date(unixtime * 1000), fmt)
}
