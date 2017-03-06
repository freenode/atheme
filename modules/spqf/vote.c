/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"


static
void spqf_vote(sourceinfo_t *const si,
               const int parc,
               char **const parv)
noexcept try
{
	if(parc < 2)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "VOTE");
		command_fail(si, fault_needmoreparams, _("Syntax: VOTE <#id> <yea|nay|veto|abstain>"));
		return;
	}

	if(!isnumeric(parv[0]))
	{
		command_fail(si, fault_badparams, _("You must specify the ID number as a number."));
		return;
	}

	const auto id(atoi(parv[0]));
	const auto bal(ballot(parv[1]));
	spqf->votes.cast(si,id,bal);
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_vote
{
	"VOTE",
	N_(N_("Cast a ballot on a motion by ID number.")),
	AC_AUTHENTICATED,
	2,
	spqf_vote,
	{ .path = "vote" }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m);
	service_bind_command(myservice,&cmd_vote);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(myservice,&cmd_vote);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/vote",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
