/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"


struct Kick : public Vote
{
	bool on_start() override
	{
		const auto target(user_find(get_issue().c_str()));
		if(!target || !chanuser_find(get_chan()->chan,target))
		{
			notice(myservice->nick,get_nick().c_str(),"Could not find that user in channel \2%s\2.",get_chan()->name);
			return false;
		}

		return true;
	}

	bool on_effect() override
	{
		const auto target(user_find(get_issue().c_str()));
		if(!target)
			return false;

		kick(myservice->me,get_chan()->chan,target,"Voted off the island");
		return true;
	}

	void on_nickchange(hook_user_nick_t *const &hook) override
	{
		set_issue(hook->u->nick);
	}

	template<class... Args>
	Kick(Args&&... args):
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
	return std::unique_ptr<Vote>(new Kick(id,type,db));
}


static
void spqf_kick(sourceinfo_t *const si,
               const int parc,
               char **const parv)
noexcept try
{
	if(parc < 2)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "KICK");
		command_fail(si, fault_needmoreparams, _("Syntax: KICK <#channel> <nickname>"));
		return;
	}

	const auto &chan(parv[0]);
	const auto &target(parv[1]);
	auto *const vote(spqf->votes.motion<Kick>(Type::KICK,chan,si,target));
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_kick
{
	"KICK",
	N_(N_("Vote to kick a user from the channel.")),
	AC_AUTHENTICATED,
	2,
	spqf_kick,
	{ .path = "kick" }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m,Type::KICK,&instance_generator);
	service_bind_command(myservice,&cmd_kick);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(myservice,&cmd_kick);
	module_fini_common(intent,Type::KICK);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/kick",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
