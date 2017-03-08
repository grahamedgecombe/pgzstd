# pgzstd

## Introduction

pgzstd is a PostgreSQL extension that provides functions for compressing and
uncompressing [Zstandard][zstd] streams, with support for custom dictionaries.

## Prerequisites

* PostgreSQL headers, libraries and PGXS build infrastructure
* `pg_config` must be in your `PATH`
* Zstandard

## Building

Run `make` to build the extension.

## Installation

Run `make install` as `root` (e.g. with `sudo`) to install the extension.

## Debian package

The repository also contains the files for building a Debian package, which can
be done by running `pg_buildext updatecontrol` followed by `dpkg-buildpackage`.
I distribute pre-built versions for stable amd64 Debian using the
[apt.postgresql.org][pgapt] repository in my [personal APT repository][apt]. Run
`apt-get install postgresql-PGVERSION-zstd` as root after setting up the
repository.

## Usage

Run `CREATE EXTENSION zstd` to install the extension in the current database.
Two functions are provided:

| Function                                                                                | Return Type |
|-----------------------------------------------------------------------------------------|-------------|
| <code>zstd\_compress(*data* bytea [, *dictionary* bytea [, *level* integer ]])</code>   | `bytea`     |
| <code>zstd\_decompress(*data* bytea [, *dictionary* bytea ])</code>                     | `bytea`     |

`zstd_compress` compresses the provided `data` and returns a Zstandard stream. A
preset `dictionary` may also be provided. The default compression `level` may
also be overriden, valid values range from `1` (best speed) to `22` (best
compression). The default level is `3`.

If you want to override the compression level without using a dictionary, set
`dictionary` to `NULL`.

`zstd_decompress` decompresses the provided Zstandard stream in `data` and
returns the uncompressed data. A preset `dictionary`, matching the dictionary
used to compress the data, may also be provided.

## Example

    gpe=# CREATE EXTENSION zstd;
    CREATE EXTENSION
	gpe=# SELECT zstd_compress('hello hello hello hello', 'hello hello', 3);
				zstd_compress
	--------------------------------------
	 \x28b52ffd2017450000000200291c6c1420
	(1 row)

	gpe=# SELECT convert_from(zstd_decompress('\x28b52ffd2017450000000200291c6c1420', 'hello hello'), 'utf-8');
		  convert_from
	-------------------------
	 hello hello hello hello
	(1 row)

	gpe=#

## License

This project is available under the terms of the ISC license, which is similar
to the 2-clause BSD license. See the `LICENSE` file for the copyright
information and licensing terms.

[zstd]: http://www.zstd.net/
[pgapt]: https://wiki.postgresql.org/wiki/Apt
[apt]: https://www.grahamedgecombe.com/apt-repository
