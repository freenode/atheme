/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"


static
void list_vote(sourceinfo_t *const &si,
               const Vote &vote)
{
	using namespace colors;

	const auto cfg(vote.get_config());
	const auto &ballots(vote.get_ballots());

	const bool voted_yea(exists<YEA>(ballots,si));
	const bool voted_nay(!voted_yea && exists<NAY>(ballots,si));
	const bool voted_veto(voted_nay && exists<VETO>(ballots,si));

	const bool fail(failed(vote.get_state()));
	const bool pass(!fail && passed(vote.get_state()));
	const bool pend(!pass && pending(vote.get_state()));
	const auto more(moreyeas(cfg,ballots,vote.get_quorum()));

	char addl[128], str[64], time[32];
	addl[0] = '\0';

	if(pend && more)
	{
		snprintf(str,sizeof(str),"\2%u\2 \x03%02umore votes\x0f are required to pass. ",more,FG::RED);
		mowgli_strlcat(addl,str,sizeof(addl));
	}
	else if(pend && !more)
	{
		snprintf(str,sizeof(str),"As it stands the \2\x03%02umotion will pass\x0f. ",FG::GREEN);
		mowgli_strlcat(addl,str,sizeof(addl));
	}

	if(pend)
	{
		secs_cast(time,sizeof(time),vote.remaining());
		snprintf(str,sizeof(time),"\2%s\2 left. ",time);
		mowgli_strlcat(addl,str,sizeof(addl));
	}
	else if(!pend)
	{
		secs_cast(time,sizeof(time),CURRTIME - vote.get_began());
		snprintf(str,sizeof(time),"\2\x03%02u%s\2 ago\x0f. ",FG::GRAY,time);
		mowgli_strlcat(addl,str,sizeof(addl));
	}

	if(pass && vote.remaining_effect())
	{
		secs_cast(time,sizeof(time),vote.remaining_effect());
		snprintf(str,sizeof(str),"\2\x03%02ueffective\x0f \2%s\2 more. ",FG::GREEN,time);
		mowgli_strlcat(addl,str,sizeof(addl));
	}

	if(fail)
	{
		snprintf(str,sizeof(str),"\2\x03%02u%s\x0f. ",FG::RED,reflect(vote.get_state()));
		mowgli_strlcat(addl,str,sizeof(addl));
	}

	command_success_nodata(si,"#\2%u\2: \2YEA\2: \2\x03%02u%-2u\x0f \2NAY\2: \2\x03%02u%-2u\x0f \2YOU\2: \2\x03%02u,%02u%s\x0f \2\x03%02u,%02u%s\x0f %s \2%s\2: \x1f%s\x0f - %s",
	                       vote.get_id(),
	                       FG::GREEN,
	                       count<YEA>(ballots),
	                       FG::RED,
	                       count<NAY>(ballots),
	                       voted_yea || voted_nay? FG::WHITE : FG::BLACK,
	                       voted_yea? BG::GREEN: voted_veto? BG::MAGENTA: voted_nay? BG::RED: pend? BG::LGRAY_BLINK : BG::LGRAY,
	                       voted_yea? "YEA": voted_nay? "NAY": "---",
	                       pass || fail? FG::WHITE: FG::BLACK,
	                       pass? BG::GREEN: fail? BG::RED: pend? BG::ORANGE: BG::BLACK,
	                       pass? "PASSED": fail? "FAILED": pend? "ACTIVE": "??????",
	                       vote.get_chan()->name,
	                       reflect(vote.get_type()),
	                       vote.get_issue().c_str(),
	                       addl);

}


static
void spqf_list(sourceinfo_t *const si,
               const int parc,
               char **const parv)
noexcept try
{
	if(parc < 1)
	{
		command_fail(si, fault_needmoreparams, _("Syntax: LIST <#channel>"));
		return;
	}

	const auto &channame(parv[0]);
	const auto chan(mychan_find(channame));
	if(!chan)
	{
		command_fail(si, fault_nosuch_target, _("This channel is not registered or doesn't exist."));
		return;
	}

	const auto &votes(spqf->votes);
	auto pit(votes.cidx.equal_range(chan));
	if(pit.first == pit.second)
	{
		command_success_nodata(si, _("This channel has no votes on record."));
		return;
	}

	size_t i(0); do
	{
		--pit.second;
		const Vote *const &vote(pit.second->second);
		list_vote(si,*vote);
	}
	while(pit.first != pit.second && ++i < 20); //TODO: limit
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_list
{
	"LIST",
	N_(N_("Listing of voting motions.")),
	AC_NONE,
	2,
	spqf_list,
	{ .path = "list" }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m);
	service_bind_command(myservice,&cmd_list);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(myservice,&cmd_list);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/list",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
