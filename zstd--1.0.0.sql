CREATE OR REPLACE FUNCTION zstd_compress(bytea, bytea DEFAULT NULL, integer DEFAULT NULL) RETURNS bytea
    AS '$libdir/zstd.so', 'compress'
    LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION zstd_decompress(bytea, bytea DEFAULT NULL) RETURNS bytea
    AS '$libdir/zstd.so', 'decompress'
    LANGUAGE C IMMUTABLE;
