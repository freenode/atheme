/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"


void spqf_help(sourceinfo_t *si, int parc, char *parv[])
noexcept try
{
	spqf->helpcmd.first = parc;
	spqf->helpcmd.second = parv;

	if(!parc)
	{
		command_success_nodata(si, _("****** \2" SPQF_TITLE "\2 ******"));
		command_success_nodata(si, _("\2%s\2 is a threshold-operator replacing the need for operators"), si->service->nick);
		command_success_nodata(si, _("to agree, be present, or even exist. A versatile configuration"));
		command_success_nodata(si, _("allows many aspects of channel management to be crowdsourced to"));
		command_success_nodata(si, _("a much larger list of semi-trusted users than would ever be"));
		command_success_nodata(si, _("practical with full privileges. \2%s\2 reduces the burden on a"), si->service->nick);
		command_success_nodata(si, _("founder to maintain adequate channel coverage balanced with a"));
		command_success_nodata(si, _("consistent user experience apropos rule enforcement."));
		command_success_nodata(si, _(""));
		command_success_nodata(si, _("To begin using \2%s\2 you must enable vote types for your channel"), si->service->nick);
		command_success_nodata(si, _("using the SET command."));
		command_success_nodata(si, _("ex: \2/msg %s SET <#channel> QUIET VOTE ENABLE 1\2"), si->service->nick);
		command_success_nodata(si, _("see also: /msg %s help SET"), si->service->nick);
		command_success_nodata(si, _(""));
		command_success_nodata(si, _("Visit us @ \2irc.freenode.net/#SPQF\2 for questions & comments."));
		command_success_nodata(si, _(""));
		command_success_nodata(si, _("For more information on a command, type:"));
		command_success_nodata(si, "\2/%s%s help <command>\2", (ircd->uses_rcommand == false) ? "msg " : "", si->service->nick);
		command_success_nodata(si, " ");

		command_help(si, si->service->commands);

		command_success_nodata(si, _("***** \2End of Help\2 *****"));
		return;
	}
	else help_display_as_subcmd(si, si->service, nullptr, parv[0], si->service->commands);
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_help
{
	"HELP",
	N_(N_("Displays contextual help information.")),
	AC_NONE,
	6,
	spqf_help,
	{ nullptr, nullptr }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m);
	service_bind_command(myservice,&cmd_help);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(myservice,&cmd_help);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/help",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
