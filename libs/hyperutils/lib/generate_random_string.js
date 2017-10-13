const crypto = require('crypto')

module.exports = n => {
    const buf = Buffer.alloc((n / 2) + 1)
    crypto.randomFillSync(buf)
    return buf.toString('hex').substring(0, n)
}