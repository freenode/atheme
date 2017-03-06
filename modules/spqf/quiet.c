/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"
#include <sstream>


struct Quiet : public Vote
{
	bool on_start() override
	{
		const auto target(user_find(get_issue().c_str()));
		if(!target || !chanuser_find(get_chan()->chan,target))
		{
			notice(myservice->nick,get_nick().c_str(),"Could not find that user in channel \2%s\2.",get_chan()->name);
			return false;
		}

		std::stringstream head, args;
		head << "+q";
		args << "*!*@" << target->vhost;
		if(target->myuser)
		{
			head << "q";
			args << " $a:" << target->myuser->ent.name;
		}

		head << " " << args.str();
		set_effect(head.str());
		return true;
	}

	bool on_effect() override
	{
		channel_mode(myservice->me,get_chan()->chan,get_effect());
		set_expires();
		return true;
	}

	bool on_expire() override
	{
		std::string effect(get_effect());
		effect.at(0) = '-';
		channel_mode(myservice->me,get_chan()->chan,effect);
		return true;
	}

	template<class... Args>
	Quiet(Args&&... args):
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
	return std::unique_ptr<Vote>(new Quiet(id,type,db));
}


static
void spqf_quiet(sourceinfo_t *const si,
                const int parc,
                char **const parv)
noexcept try
{
	if(parc < 2)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "QUIET");
		command_fail(si, fault_needmoreparams, _("Syntax: QUIET <#channel> <nickname|hostmask> [...]"));
		return;
	}

	const auto &chan(parv[0]);
	const auto &target(parv[1]);
	auto *const vote(spqf->votes.motion<Quiet>(Type::QUIET,chan,si,target));
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_quiet
{
	"QUIET",
	N_(N_("Vote to set a quiet on a channel.")),
	AC_AUTHENTICATED,
	2,
	spqf_quiet,
	{ .path = "quiet" }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m,Type::QUIET,&instance_generator);
	service_bind_command(myservice,&cmd_quiet);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(myservice,&cmd_quiet);
	module_fini_common(intent,Type::QUIET);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/quiet",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
