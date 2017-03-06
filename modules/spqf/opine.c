/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"


struct Opine : public Vote
{
	bool on_passed() override
	{
		notice(myservice->nick,get_chan()->name,
		       "The people of %s have decided: \2%s\2",
		       get_chan()->name,
		       get_issue().c_str());

		return true;
	}

	template<class... Args>
	Opine(Args&&... args):
	Vote(std::forward<Args>(args)...)
	{
	}
};


static
std::unique_ptr<Vote>
instance_generator(const uint &id,
                   const Type &type,
                   database_handle_t *const &db)
{
	return std::unique_ptr<Vote>(new Opine(id,type,db));
}


static
void spqf_opine(sourceinfo_t *const si,
                const int parc,
                char **const parv)
noexcept try
{
	if(parc < 2)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "OPINE");
		command_fail(si, fault_needmoreparams, _("Syntax: OPINE <#channel> [opinion]"));
		return;
	}

	const auto &chan(parv[0]);
	const auto &opinion(parv[1]);
	auto *const vote(spqf->votes.motion<Opine>(Type::OPINE,chan,si,opinion));
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_opine
{
	"OPINE",
	N_(N_("Vote on an opinion with no effects.")),
	AC_AUTHENTICATED,
	2,
	spqf_opine,
	{ .path = "opine" }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m,Type::OPINE,&instance_generator);
	service_bind_command(myservice,&cmd_opine);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(myservice,&cmd_opine);
	module_fini_common(intent,Type::OPINE);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/opine",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
