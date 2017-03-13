#include <postgres.h>
#include <fmgr.h>
#include <zstd.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#ifndef PG_INT32_MAX
#define PG_INT32_MAX 0x7fffffff
#endif

#define DEFAULT_COMPRESSION_LEVEL 3

static ZSTD_CCtx *cctx;
static ZSTD_DCtx *dctx;

void _PG_init(void);
void _PG_fini(void);
Datum compress(PG_FUNCTION_ARGS);
Datum decompress(PG_FUNCTION_ARGS);
Datum length(PG_FUNCTION_ARGS);

void _PG_init(void)
{
    cctx = ZSTD_createCCtx();
    if (!cctx)
        elog(FATAL, "ZSTD_createCCtx failed");

    dctx = ZSTD_createDCtx();
    if (!dctx)
        elog(FATAL, "ZSTD_createDCtx failed");
}

void _PG_fini(void)
{
    if (cctx)
        ZSTD_freeCCtx(cctx);

    if (dctx)
        ZSTD_freeDCtx(dctx);
}

PG_FUNCTION_INFO_V1(compress);
Datum compress(PG_FUNCTION_ARGS)
{
    bytea *in, *out;
    char *dict = NULL;
    int32 level;
    size_t in_len, out_len, dict_len = 0;

    if (PG_ARGISNULL(0))
        PG_RETURN_NULL();

    in = PG_GETARG_BYTEA_P(0);
    in_len = VARSIZE(in) - VARHDRSZ;

    level = PG_ARGISNULL(2) ? DEFAULT_COMPRESSION_LEVEL : PG_GETARG_INT32(2);

    if (!PG_ARGISNULL(1))
    {
        bytea *d = PG_GETARG_BYTEA_P(1);
        dict = VARDATA(d);
        dict_len = VARSIZE(d) - VARHDRSZ;
    }

    out_len = ZSTD_compressBound(in_len);
    out = palloc(out_len + VARHDRSZ);

    out_len = ZSTD_compress_usingDict(cctx, VARDATA(out), out_len, VARDATA(in), in_len, dict, dict_len, level);

    if (ZSTD_isError(out_len))
        elog(ERROR, "ZSTD_compress_usingDict failed: %s", ZSTD_getErrorName(out_len));

    out = repalloc(out, out_len + VARHDRSZ);

    SET_VARSIZE(out, out_len + VARHDRSZ);
    PG_RETURN_BYTEA_P(out);
}

PG_FUNCTION_INFO_V1(decompress);
Datum decompress(PG_FUNCTION_ARGS)
{
    bytea *in, *out;
    char *dict = NULL;
    size_t in_len, out_len, dict_len = 0;

    if (PG_ARGISNULL(0))
        PG_RETURN_NULL();

    in = PG_GETARG_BYTEA_P(0);
    in_len = VARSIZE(in) - VARHDRSZ;

    if (!PG_ARGISNULL(1))
    {
        bytea *d = PG_GETARG_BYTEA_P(1);
        dict = VARDATA(d);
        dict_len = VARSIZE(d) - VARHDRSZ;
    }

    /*
     * XXX this function returns 0 in two cases: if the decompressed size really
     * is zero, or if there was an error finding the decompressed size. Future
     * versions of zstd provide ZSTD_getFrameContentSize(), which distinguishes
     * between zero length and an error. However, we're stuck with
     * ZSTD_getDecompressedSize() for now and always assume zero means a
     * decompressed size of zero.
     */
    out_len = ZSTD_getDecompressedSize(VARDATA(in), in_len);

    out = palloc(out_len + VARHDRSZ);

    out_len = ZSTD_decompress_usingDict(dctx, VARDATA(out), out_len, VARDATA(in), in_len, dict, dict_len);

    if (ZSTD_isError(out_len))
        elog(ERROR, "ZSTD_decompress_usingDict failed: %s", ZSTD_getErrorName(out_len));

    SET_VARSIZE(out, out_len + VARHDRSZ);
    PG_RETURN_BYTEA_P(out);
}

PG_FUNCTION_INFO_V1(length);
Datum length(PG_FUNCTION_ARGS)
{
    bytea *in;
    size_t in_len, out_len;

    if (PG_ARGISNULL(0))
        PG_RETURN_NULL();

    in = PG_GETARG_BYTEA_P(0);
    in_len = VARSIZE(in) - VARHDRSZ;

    /* XXX see above */
    out_len = ZSTD_getDecompressedSize(VARDATA(in), in_len);
    if (out_len > PG_INT32_MAX)
        elog(ERROR, "ZSTD_getDecompressedSize returned value greater than PG_INT32_MAX");

    PG_RETURN_INT32((int32) out_len);
}
