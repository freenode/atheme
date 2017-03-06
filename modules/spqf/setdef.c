/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"
#include "param.h"


static
void spqf_setdef(sourceinfo_t *const si,
                 const int parc,
                 char **const parv)
noexcept try
{
	if(parc < 3)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "SETDEF");
		command_fail(si, fault_needmoreparams, _("Syntax: SETDEF <vote type> <category> <parameter> <value>"));
		return;
	}

	const auto &typestr(parv[0]);
	if(!exists<Type>(typestr))
	{
		command_fail(si, fault_badparams, _("This vote type is not valid."));
		return;
	}

	const auto &docstr(parv[1]);
	if(!exists<Doc>(docstr))
	{
		command_fail(si, fault_badparams, _("No such parameter category."));
		return;
	}

	const auto &varstr(parv[2]);
	if(!exists<Var>(varstr))
	{
		command_fail(si, fault_badparams, _("Not a valid parameter name."));
		return;
	}

	const auto type(reflect<Type>(typestr));
	const auto doc(reflect<Doc>(docstr));
	const auto var(reflect<Var>(varstr));
	const auto &value(parc > 3? parv[3] : nullptr);

	char errstr[BUFSIZE] {0};
	if(!param::valid(doc,var,value,errstr,sizeof(errstr)))
	{
		command_fail(si, fault_badparams, "%s", errstr);
		return;
	}

	Metadata md(metadata,Cfg::OPERDEF_PREFIX);
	md.set({uint(type),uint(doc),uint(var)},value);

	command_success_nodata(si, _("\2%s\2 parameter \2%s\2 \2%s\2 for vote type \2%s\2."),
	                       value? "UPDATED" : "UNSET",
	                       reflect(doc),
	                       reflect(var),
	                       reflect(type));
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


command_t cmd_setdef
{
	"SETDEF",
	N_(N_("Configure default parameters for all channels.")),
	PRIV_USER_AUSPEX,
	4,
	spqf_setdef,
	{ nullptr, nullptr }
};


void module_init(module_t *const m)
noexcept
{
	module_init_common(m);
	service_bind_command(spqf->svc,&cmd_setdef);
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	service_unbind_command(spqf->svc,&cmd_setdef);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/setdef",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
