/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"
#include "param.h"


static
void info_chan_config(sourceinfo_t *const si,
                      const int parc,
                      char **const parv,
                      mychan_t *const &chan)
{
	if(parc < 3 || (parc >= 3 && !exists<Type>(parv[2])))
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "INFO");
		command_fail(si, fault_needmoreparams, _("Must specify a valid vote type, or DEFAULT."));
		return;
	}

	const auto type(reflect<Type>(parv[2]));
	command_success_nodata(si,"\2***\2 Configuration for channel \2%s\2 \2***\2",
	                       chan->name);

	const Cfg cfg(chan,type);
	for(size_t i(0); i < num_of<Doc>(); ++i)
	{
		const auto &params(param::tree[i]);
		const auto doc(static_cast<Doc>(i));
		for(const auto &p : params)
		{
			const auto &var(get<param::VAR>(p));
			const auto val(cfg.get(doc,var));
			if(!val && type != Type::DEFAULT)
				continue;

			command_success_nodata(si,"%s: %s \2%-12s\2 \2%-16s\2 : %s",
			                       chan->name,
			                       reflect(type),
			                       reflect(doc),
			                       reflect(var),
			                       val?: "<default>");
		}
	}

	command_success_nodata(si,"\2***\2 End of configuration \2***\2");
}


static
void info_chan(sourceinfo_t *const si,
               const int parc,
               char **const parv)
{
	auto *const chan(mychan_find(parv[0]));
	if(!chan)
	{
		command_fail(si, fault_nosuch_target, _("Channel is not registered."));
		return;
	}

	if(parc > 1) switch(hash(parv[1]))
	{
		case hash("config"):
			info_chan_config(si,parc,parv,chan);
			return;

		default:
			command_fail(si, fault_badparams, _("Not a valid subcommand for info about \2%s\2"), chan->name);
			return;
	}

	#define INFO_LINE "%s: \2%-13s:\2 "

	{
		char buf[BUFSIZE];
		buf[0] = '\0';
		for_each<Type>([&chan,&buf](const Type &type)
		{
			const Cfg cfg(chan,type);
			if(cfg.get<bool>(Doc::VOTE,Var::ENABLE,false))
			{
				mowgli_strlcat(buf,reflect(type),sizeof(buf));
				mowgli_strlcat(buf," ",sizeof(buf));
			}
		});

		command_success_nodata(si,INFO_LINE"%s",
		                          chan->name,
		                          "ENABLED",
		                          buf);
	}

	command_success_nodata(si,INFO_LINE"%zu",
	                          chan->name,
	                          "TOTAL VOTES",
	                          spqf->votes.count(chan));
	#undef INFO_LINE
}


static
void info_vote(sourceinfo_t *const si,
               const int parc,
               char **const parv)
{
	const auto id(atoi(parv[0]));
	const auto *const vote(spqf->votes.get(id));
	if(!vote)
	{
		command_fail(si, fault_nosuch_key, _("Failed to find a vote with that ID number."));
		return;
	}

	char status[64] {0};
	const bool pend(pending(vote->get_state()));
	snprintf(status,sizeof(status),
	         "\x02\x03%02u%s\x0f",
	         pend? 3 : 5,
	         pend? "ACTIVE" : "CLOSED");

	command_success_nodata(si,"#%u: Information on vote #\2%u\2 %s",
	                       vote->get_id(),
	                       vote->get_id(),
	                       status);

	#define INFO_LINE "#%u: \2%-10s:\2 "

	command_success_nodata(si,INFO_LINE"%s",
	                       vote->get_id(),
	                       "TYPE",
	                       reflect(vote->get_type()));

	command_success_nodata(si,INFO_LINE"%s",
	                       vote->get_id(),
	                       "ISSUE",
	                       vote->get_issue().c_str());

	command_success_nodata(si,INFO_LINE"%s",
	                       vote->get_id(),
	                       "CHANNEL",
	                       vote->get_chan()->name);

	command_success_nodata(si,INFO_LINE"%s",
	                       vote->get_id(),
	                       "SPEAKER",
	                       vote->get_acct().c_str());

	if(vote->get_began())
	{
		struct tm tm {0};
		char timebuf[64] {0};
		const time_t began(vote->get_began());
		gmtime_r(&began,&tm);
		strftime(timebuf,sizeof(timebuf),"%c",&tm);
		command_success_nodata(si,INFO_LINE"%ld (%s)",
		                       vote->get_id(),
		                       "STARTED",
		                       began,
		                       timebuf);
	}

	if(vote->get_ending())
	{
		struct tm tm {0};
		char timebuf[64] {0};
		const time_t ending(vote->get_ending());
		gmtime_r(&ending,&tm);
		strftime(timebuf,sizeof(timebuf),"%c",&tm);
		command_success_nodata(si,INFO_LINE"%ld (%s)",
		                       vote->get_id(),
		                       "ENDING",
		                       ending,
		                       timebuf);
	}

	if(vote->get_expires())
	{
		struct tm tm {0};
		char timebuf[64] {0};
		const time_t expires(vote->get_expires());
		gmtime_r(&expires,&tm);
		strftime(timebuf,sizeof(timebuf),"%c",&tm);
		command_success_nodata(si,INFO_LINE"%ld (%s)",
		                       vote->get_id(),
		                       "EXPIRES",
		                       expires,
		                       timebuf);
	}

	if(!vote->get_effect().empty())
		command_success_nodata(si,INFO_LINE"%s",
		                       vote->get_id(),
		                       "EFFECTS",
		                       vote->get_effect().c_str());

	if(vote->get_quorum() > 0)
		command_success_nodata(si,INFO_LINE"%u",
		                       vote->get_id(),
		                       "QUORUM",
	                           vote->get_quorum());

	const auto cfg(vote->get_config());
	const auto &ballots(vote->get_ballots());
	const bool vis_tally(cfg.get<bool>(Doc::VISIBLE,Var::TALLY,true));
	const bool vis_pend(cfg.get<bool>(Doc::VISIBLE,Var::PENDING,true));
	const bool vis_any(vis_tally && (vis_pend || !pend));

	if(!vis_tally)
	{
		command_success_nodata(si,INFO_LINE"The tally for this vote is secret.",
		                       vote->get_id(),
		                       "VISIBILITY");
	}
	else if(!vis_pend && pend)
	{
		command_success_nodata(si,INFO_LINE"The tally for this vote is secret until voting has ended.",
		                       vote->get_id(),
		                       "VISIBILITY");
	}

	// YEA line
	if(vis_any && !get<YEA>(ballots).empty())
	{
		const bool vis_bal(cfg.get<bool>(Doc::VISIBLE,Var::YEAS,true));
		command_success_nodata(si,INFO_LINE"%u - %s",
		                       vote->get_id(),
		                       "YEA",
		                       count<YEA>(ballots),
		                       vis_bal? get<YEA>(ballots).c_str() : "<Ballots for this vote are secret>");
	}

	// NAY line
	if(vis_any && !get<NAY>(ballots).empty())
	{
		const bool vis_bal(cfg.get<bool>(Doc::VISIBLE,Var::NAYS,true));
		command_success_nodata(si,INFO_LINE"%u - %s",
		                       vote->get_id(),
		                       "NAY",
		                       count<NAY>(ballots),
		                       vis_bal? get<NAY>(ballots).c_str() : "<Ballots for this vote are secret>");
	}

	// VETO line
	if(vis_any && !get<VETO>(ballots).empty())
	{
		const bool vis_bal(cfg.get<bool>(Doc::VISIBLE,Var::VETOES,true));
		command_success_nodata(si,INFO_LINE"%u - %s",
		                       vote->get_id(),
		                       "VETO",
		                       count<VETO>(ballots),
		                       vis_bal? get<VETO>(ballots).c_str() : "<Ballots for this vote are secret>");
	}

	command_success_nodata(si,INFO_LINE"%s",
	                       vote->get_id(),
	                       "STATUS",
	                       reflect(vote->get_state()));

	command_success_nodata(si,"#%u: *** End of Info ***",vote->get_id());

	#undef INFO_LINE
}


static
void spqf_info(sourceinfo_t *const si,
               const int parc,
               char **const parv)
noexcept try
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "INFO");
		command_fail(si, fault_needmoreparams, _("Syntax: INFO < vote id | <#channel> [config <type>] >"));
		return;
	}

	const auto &idstr(parv[0]);
	if(isnumeric(idstr))
		info_vote(si,parc,parv);
	else
		info_chan(si,parc,parv);
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_info
{
	"INFO",
	N_(N_("Display information about a vote by id.")),
	AC_NONE,
	3,
	spqf_info,
	{ .path = "info" }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m);
	service_bind_command(myservice,&cmd_info);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(myservice,&cmd_info);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/info",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
