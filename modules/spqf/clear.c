/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"


static
void spqf_clear(sourceinfo_t *const si,
                const int parc,
                char **const parv)
noexcept try
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, _("Syntax: CLEAR <#channel>"));
		return;
	}

	const auto &channame(parv[0]);
	const auto chan(mychan_find(channame));
	if(!chan)
	{
		command_fail(si, fault_nosuch_target, _("This channel is not registered or doesn't exist."));
		return;
	}

	const auto uflags(chanacs_source_flags(chan,si));
	if(~uflags & CA_SET && ~uflags & CA_FOUNDER)
	{
		command_fail(si, fault_authfail, _("You are not authorized to use the \2CLEAR\2 command on \2%s\2."),chan->name);
		return;
	}

	auto &votes(spqf->votes);
	const size_t cleared(votes.erase(chan));
	command_success_nodata(si, _("Cleared \2%zu\2 votes from \2%s\2."), cleared, chan->name);
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_clear
{
	"CLEAR",
	N_(N_("Clear votes for a channel.")),
	AC_AUTHENTICATED,
	1,
	spqf_clear,
	{ .path = "clear" }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m);
	service_bind_command(spqf->svc, &cmd_clear);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(spqf->svc, &cmd_clear);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/clear",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
