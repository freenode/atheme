/* groupserv - group services.
 * Copyright (c) 2010 Atheme Development Group
 */

#include "groupserv.h"

static void gs_cmd_help(sourceinfo_t *si, int parc, char *parv[]);

command_t gs_help = { "HELP", N_("Displays contextual help information."), AC_NONE, 2, gs_cmd_help };

void gs_cmd_help(sourceinfo_t *si, int parc, char *parv[])
{
	if (!parv[0])
	{
		command_success_nodata(si, _("***** \2%s Help\2 *****"), si->service->nick);
		command_success_nodata(si, _("\2%s\2 provides tools for managing groups of users and channels."), si->service->nick);
		command_success_nodata(si, " ");
		command_success_nodata(si, _("For more information on a command, type:"));
		command_success_nodata(si, "\2/%s%s help <command>\2", (ircd->uses_rcommand == false) ? "msg " : "", si->service->disp);
		command_success_nodata(si, " ");

		command_help(si, &gs_cmdtree);

		command_success_nodata(si, _("***** \2End of Help\2 *****"));
		return;
	}

	/* take the command through the hash table */
	help_display(si, si->service, parv[0], &gs_helptree);
}

static void gs_cmd_register(sourceinfo_t *si, int parc, char *parv[]);

command_t gs_register = { "REGISTER", N_("Registers a group."), AC_NONE, 2, gs_cmd_register };

static void gs_cmd_register(sourceinfo_t *si, int parc, char *parv[])
{
	mygroup_t *mg;

	if (!parv[0])
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "REGISTER");
		command_fail(si, fault_needmoreparams, _("To register a group: REGISTER <!groupname>"));
		return;
	}

	if (*parv[0] != '!')
	{
		command_fail(si, fault_needmoreparams, STR_INVALID_PARAMS, "REGISTER");
		command_fail(si, fault_needmoreparams, _("To register a group: REGISTER <!groupname>"));
		return;
	}

	if ((mg = mygroup_find(parv[0])) != NULL)
	{
		command_fail(si, fault_alreadyexists, _("The group \2%s\2 already exists."), parv[0]);
		return;
	}

	if (si->smu == NULL)
	{
		command_fail(si, fault_noprivs, _("You are not logged in."));
		return;
	}

	/* XXX: need to check registration limits here. */
	mg = mygroup_add(parv[0]);
	node_add(si->smu, node_create(), &mg->acs);

	command_success_nodata(si, _("The group \2%s\2 has been registered to \2%s\2."), entity(mg)->name, entity(si->smu)->name);
}

void basecmds_init(void)
{
	command_add(&gs_help, &gs_cmdtree);
	command_add(&gs_register, &gs_cmdtree);

	help_addentry(&gs_helptree, "HELP", "help/help", NULL);
}

void basecmds_deinit(void)
{
	command_delete(&gs_help, &gs_cmdtree);
	command_delete(&gs_register, &gs_cmdtree);

	help_delentry(&gs_helptree, "HELP");
}
