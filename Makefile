MODULE_big = zstd
EXTENSION = zstd
OBJS = zstd.o
DATA = zstd--1.0.0.sql

SHLIB_LINK += -lzstd

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
