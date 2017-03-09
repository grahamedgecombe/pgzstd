CREATE OR REPLACE FUNCTION zstd_length(bytea) RETURNS integer
    AS '$libdir/zstd.so', 'length'
    LANGUAGE C IMMUTABLE;
