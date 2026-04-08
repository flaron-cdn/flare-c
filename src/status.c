#include "flare/status.h"

const char *flare_status_str(flare_status_t status) {
    switch (status) {
    case FLARE_OK:                       return "ok";
    case FLARE_ERR_NOT_FOUND:            return "not found";
    case FLARE_ERR_INVALID_ARG:          return "invalid argument";
    case FLARE_ERR_OUT_OF_MEMORY:        return "out of memory";
    case FLARE_ERR_HOST:                 return "host error";
    case FLARE_ERR_DECODE:               return "decode error";
    case FLARE_ERR_NO_RESPONSE:          return "no response from host";
    case FLARE_ERR_SPARK_INVALID_TTL:    return "spark: invalid ttl";
    case FLARE_ERR_SPARK_TOO_LARGE:      return "spark: value too large";
    case FLARE_ERR_SPARK_WRITE_LIMIT:    return "spark: write limit reached";
    case FLARE_ERR_SPARK_QUOTA:          return "spark: quota exceeded";
    case FLARE_ERR_SPARK_NOT_AVAIL:      return "spark: not available";
    case FLARE_ERR_SPARK_INTERNAL:       return "spark: internal error";
    case FLARE_ERR_SPARK_READ_LIMIT:     return "spark: read limit reached";
    case FLARE_ERR_SPARK_BAD_KEY:        return "spark: bad key";
    case FLARE_ERR_SPARK_NO_CAPAB:       return "spark: no write capability";
    case FLARE_ERR_PLASMA_NOT_AVAIL:     return "plasma: not available";
    case FLARE_ERR_PLASMA_WRITE_LIMIT:   return "plasma: write limit reached";
    case FLARE_ERR_PLASMA_TOO_LARGE:     return "plasma: value too large";
    case FLARE_ERR_PLASMA_BAD_KEY:       return "plasma: bad key";
    case FLARE_ERR_PLASMA_NO_CAPAB:      return "plasma: no write capability";
    case FLARE_ERR_PLASMA_INTERNAL:      return "plasma: internal error";
    case FLARE_ERR_WS_SEND:              return "ws: send failed";
    case FLARE_ERR_WS_CLOSED:            return "ws: connection closed";
    }
    return "unknown";
}
