# Runtime configuration and operations

Atlas expects a working MariaDB/MySQL instance and a writable runtime directory.

## Startup behavior

When launched, the server:

1. checks for `config.lua`; if absent, copies `config.lua.dist` to `config.lua`;
2. loads runtime settings from `config.lua`;
3. loads the RSA key from `key.pem`;
4. connects to the configured database;
5. verifies the database schema is present; and
6. initializes the game, scripts, and networking layers.

The main startup logic is implemented in [src/otserv.cpp](../src/otserv.cpp).

## Primary runtime files

- [config.lua.dist](../config.lua.dist) contains the default runtime settings.
- [key.pem](../key.pem) contains the RSA key used by the server.
- [schema.sql](../schema.sql) is the expected database schema.
- [data/](../data) holds the world content, scripts, XML files, and migrations.

## Important configuration areas

The default configuration template covers:

- combat, world rules, rates, and experience settings;
- network ports for the game and status servers;
- MySQL connection settings;
- house, market, and stamina behavior;
- startup behavior and database optimization.

Review the comments in `config.lua.dist` before changing production values.

## Database and migrations

The database layer uses the schema in [schema.sql](../schema.sql). The migration scripts in [data/migrations/](../data/migrations) update the database structure over time.

When the server starts, it checks whether the configured database already contains the expected tables and will refuse to start if the schema is missing.

## HTTP support

If `ENABLE_HTTP=ON`, Atlas can compile the optional HTTP server layer from [src/http/](../src/http). The default build enables this path.
