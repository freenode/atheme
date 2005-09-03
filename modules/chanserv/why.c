/*
 * Copyright (c) 2005 William Pitcock
 * Rights to this code are as documented in doc/LICENSE.
 *
 * This file contains code for the NickServ MYACCESS function.
 *
 * $Id: why.c 2069 2005-09-03 02:38:43Z nenolod $
 */

#include "atheme.h"

DECLARE_MODULE_V1("chanserv/why", FALSE, _modinit, _moddeinit);

static void cs_cmd_why(char *origin);

command_t cs_why = {
	"WHY",
	"Explains channel access logic.",
	AC_NONE,
	cs_cmd_why
};

list_t *cs_cmdtree;

void _modinit(module_t *m)
{
	cs_cmdtree = module_locate_symbol("chanserv/main", "cs_cmdtree");
	command_add(&cs_why, cs_cmdtree);
}

void _moddeinit()
{
	command_delete(&cs_why, cs_cmdtree);
}

static void cs_cmd_why(char *origin)
{
	char *chan = strtok(NULL, " ");
	char *targ = strtok(NULL, " ");
	char host[BUFSIZE];
	mychan_t *mc = mychan_find(chan);
	user_t *u = user_find(targ);
	user_t *ou = user_find(origin);
	myuser_t *mu = u->myuser;
	node_t *n;
	chanacs_t *ca;
	uint32_t i, matches = 0;

	if (mc == NULL)
	{
		notice(chansvs.nick, origin, "\2%s\2 is not registered.",
			chan);
		return;
	}

	if (mu == NULL)
	{
		notice(chansvs.nick, origin, "\2%s\2 is not registered.",
			chan);
		return;
	}

	snprintf(host, BUFSIZE, "%s!%s@%s", u->nick, u->user, u->vhost);

	LIST_FOREACH(n, mc->chanacs.head)
	{
       	        ca = (chanacs_t *)n->data;

		if (ca->myuser != mu && match(ca->host, host))
			continue;

		if ((ca->level & CA_AUTOVOICE) == CA_AUTOVOICE)
			notice(chansvs.nick, origin,
				"\2%s\2 was granted voice in \2%s\2, because they have the autovoice privilege.", 
				targ, chan);

		if ((ca->level & CA_AUTOHALFOP) == CA_AUTOHALFOP)
			notice(chansvs.nick, origin,
				"\2%s\2 was granted halfops in \2%s\2, because they have the autohalfop privilege.",
				targ, chan);

		if ((ca->level & CA_AUTOOP) == CA_AUTOOP)
			notice(chansvs.nick, origin,
				"\2%s\2 was granted channel ops in \2%s\2, because they have the autoop privilege.",
				targ, chan);

		if ((ca->level & CA_AKICK) == CA_AKICK)
			notice(chansvs.nick, origin,
				"\2%s\2 was kicked from \2%s\2, because they are on the channel autokick list.",
				targ, chan);
	}
}
