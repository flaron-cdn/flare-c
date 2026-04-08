#ifndef FLARE_STATUS_H
#define FLARE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FLARE_OK = 0,

    FLARE_ERR_NOT_FOUND     = 1,
    FLARE_ERR_INVALID_ARG   = 2,
    FLARE_ERR_OUT_OF_MEMORY = 3,
    FLARE_ERR_HOST          = 4,
    FLARE_ERR_DECODE        = 5,
    FLARE_ERR_NO_RESPONSE   = 6,

    FLARE_ERR_SPARK_INVALID_TTL  = 101,
    FLARE_ERR_SPARK_TOO_LARGE    = 102,
    FLARE_ERR_SPARK_WRITE_LIMIT  = 103,
    FLARE_ERR_SPARK_QUOTA        = 104,
    FLARE_ERR_SPARK_NOT_AVAIL    = 105,
    FLARE_ERR_SPARK_INTERNAL     = 106,
    FLARE_ERR_SPARK_READ_LIMIT   = 107,
    FLARE_ERR_SPARK_BAD_KEY      = 108,
    FLARE_ERR_SPARK_NO_CAPAB     = 109,

    FLARE_ERR_PLASMA_NOT_AVAIL   = 201,
    FLARE_ERR_PLASMA_WRITE_LIMIT = 202,
    FLARE_ERR_PLASMA_TOO_LARGE   = 203,
    FLARE_ERR_PLASMA_BAD_KEY     = 204,
    FLARE_ERR_PLASMA_NO_CAPAB    = 205,
    FLARE_ERR_PLASMA_INTERNAL    = 206,

    FLARE_ERR_WS_SEND  = 301,
    FLARE_ERR_WS_CLOSED = 302,
} flare_status_t;

const char *flare_status_str(flare_status_t status);

#ifdef __cplusplus
}
#endif

#endif
