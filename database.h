#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "server.h"
#include "configuration.h"

int init_db(struct config *c);
int connect_db(struct config *c);
struct server **db_create_servers(struct config *c);
int db_create_channels(struct config *c, struct server *s);
int db_create_registrations(struct config *c, struct server *s);

#endif