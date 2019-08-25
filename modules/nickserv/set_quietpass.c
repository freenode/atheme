/*
 * Copyright (c) 2005 William Pitcock <nenolod -at- nenolod.net>
 * Copyright (c) 2007 Jilles Tjoelker
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Allows you to opt-out of channel change messages.
 *
 */

#include "atheme.h"
#include "uplink.h"
#include "list_common.h"
#include "list.h"

DECLARE_MODULE_V1
(
	"nickserv/set_quietpass", false, _modinit, _moddeinit,
	PACKAGE_STRING,
	"Atheme Development Group <http://www.atheme.org>"
);

mowgli_patricia_t **ns_set_cmdtree;

static void ns_cmd_set_quietpass(sourceinfo_t *si, int parc, char *parv[]);

command_t ns_set_quietpass = { "QUIETPASS", N_("Opt-out of bad password messages from others."), AC_NONE, 1, ns_cmd_set_quietpass, { .path = "nickserv/set_quietpass" } };

static bool has_quietpass(const mynick_t *mn, const void *arg)
{
	myuser_t *mu = mn->owner;

	return ( mu->flags & MU_QUIETPASS ) == MU_QUIETPASS;
}

void _modinit(module_t *m)
{
	MODULE_TRY_REQUEST_SYMBOL(m, ns_set_cmdtree, "nickserv/set_core", "ns_set_cmdtree");

	command_add(&ns_set_quietpass, *ns_set_cmdtree);

	use_nslist_main_symbols(m);

	static list_param_t quietpass;
	quietpass.opttype = OPT_BOOL;
	quietpass.is_match = has_quietpass;

	list_register("quietpass", &quietpass);
}

void _moddeinit(module_unload_intent_t intent)
{
	command_delete(&ns_set_quietpass, *ns_set_cmdtree);

	list_unregister("quietpass");
}

/* SET QUIETPASS [ON|OFF] */
static void ns_cmd_set_quietpass(sourceinfo_t *si, int parc, char *parv[])
{
	char *setting = parv[0];

	if (!setting)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "QUIETPASS");
		return;
	}

	if (!strcasecmp("ON", setting))
	{
		if (MU_QUIETPASS & si->smu->flags)
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is already set for account \2%s\2."), "QUIETPASS", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:QUIETPASS:ON");

		si->smu->flags |= MU_QUIETPASS;

		command_success_nodata(si, _("The \2%s\2 flag has been set for account \2%s\2."), "QUIETPASS" ,entity(si->smu)->name);

		return;
	}
	else if (!strcasecmp("OFF", setting))
	{
		if (!(MU_QUIETPASS & si->smu->flags))
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is not set for account \2%s\2."), "QUIETPASS", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:QUIETPASS:OFF");

		si->smu->flags &= ~MU_QUIETPASS;

		command_success_nodata(si, _("The \2%s\2 flag has been removed for account \2%s\2."), "QUIETPASS", entity(si->smu)->name);

		return;
	}
	else
	{
		command_fail(si, fault_badparams, STR_INVALID_PARAMS, "QUIETPASS");
		return;
	}
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:ts=8
 * vim:sw=8
 * vim:noexpandtab
 */
