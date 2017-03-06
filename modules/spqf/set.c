/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"
#include "param.h"


std::array<command_t, num_of<Doc>()> docs_cmd {};
std::array<command_t, num_of<Type>()> type_cmd {};
std::array<std::array<command_t, num_of<Var>()>, num_of<Doc>()> doc_cmd {};

mowgli_patricia_t *docs_tree;
mowgli_patricia_t *type_tree;
std::array<mowgli_patricia_t *, num_of<Doc>()> doc_tree {};



static
void update_type_tree()
{
	for_each<Type>([](const Type &type)
	{
		if(type == Type::DEFAULT)
			return;

		const bool is_enable(enabled(type));
		const bool exists(command_find(type_tree,reflect(type)));

		if(is_enable && !exists)
			command_add(&type_cmd[uint(type)], type_tree);
		else if(!is_enable && exists)
			command_delete(&type_cmd[uint(type)], type_tree);
	});
}


static
void spqf_set_help(sourceinfo_t *const si,
                   const char *const subcmd)
noexcept try
{
	const auto &parc(spqf->helpcmd.first);
	const auto &parv(spqf->helpcmd.second);

	if(parc < 2)
	{
		update_type_tree();
		command_help(si, type_tree);
		return;
	}

	// the part that says SET
	const auto &setstr(parv[0]);

	const auto &typestr(parv[1]);   // name of the parameter document
	if(!exists<Type>(typestr))
	{
		command_fail(si, fault_badparams, _("No such vote type is available. Try DEFAULT to apply to all types."));
		return;
	}

	const auto type(reflect<Type>(typestr));
	if(type != Type::DEFAULT && !spqf->factory.enabled(type))
	{
		command_fail(si, fault_badparams, _("This vote type is not enabled."));
		return;
	}

	if(parc < 3)
	{
		command_help(si, docs_tree);
		return;
	}

	const auto &docstr(parv[2]);   // name of the parameter document
	if(!exists<Doc>(docstr))
	{
		command_fail(si, fault_badparams, _("No such parameter category."));
		return;
	}

	const auto doc(reflect<Doc>(docstr));
	if(parc < 4)
	{
		command_help(si, doc_tree[uint(doc)]);
		return;
	}

	const auto &varstr(parv[3]);   // name of the parameter
	if(!exists<Var>(varstr))
	{
		command_fail(si, fault_badparams, _("Not a valid parameter name."));
		return;
	}

	const auto var(reflect<Var>(varstr));
	const auto &spec(param::find(doc,var));
	if(spec == param::nullspec)
	{
		command_fail(si, fault_badparams, _("A specification for this parameter was not found."));
		return;
	}

	command_success_nodata(si, _("Extended help for this is not yet available."));

	const Cfg cfg(metadata,type);
	//TODO: show ircop defaults and limits
	switch(get<param::Spec::FORM>(spec))
	{
		case param::Form::BOOLEAN:
		{
			command_success_nodata(si, _("\2**\2 Requires a boolean value i.e \"1\", \"true\" or \"0\" \"false\" (without the quotes)."));
			break;
		}

		case param::Form::UNSIGNED:
		{
			command_success_nodata(si, _("\2**\2 This value requires a positive integer."));
			if(get<param::Spec::HIGH>(spec))
				command_success_nodata(si, _("\2++\2 This specific value must be between \2%d\2 and \2%d\2 inclusively"),
				                       get<param::Spec::LOW>(spec),
				                       get<param::Spec::HIGH>(spec));

			break;
		}

		case param::Form::TIMESTR:
		{
			command_success_nodata(si, _("\2**\2 This is a time duration, requiring a positive integer optionally followed by a postfix."));
			command_success_nodata(si, _("\2**\2 i.e 10m for ten minutes, or 120 for two minutes, or 1M for one month, etc"));
			if(get<param::Spec::HIGH>(spec))
			{
				char lowbuf[32], highbuf[32];
				secs_cast(lowbuf,sizeof(lowbuf),get<param::Spec::LOW>(spec));
				secs_cast(highbuf,sizeof(highbuf),get<param::Spec::HIGH>(spec));
				command_success_nodata(si, _("\2++\2 This specific duration must be between \2%s\2 and \2%s\2 inclusively."), lowbuf, highbuf);
			}

			break;
		}

		case param::Form::CMODES:
		{
			command_success_nodata(si, _("\2**\2 A string containing any or all of o, v, or h for operator, voice, or halfop respectively."));
			command_success_nodata(si, _("\2**\2 If the user has \2any\2 one of the modes specified it is considered a match."));
			break;
		}

		case param::Form::ACSFLAGS:
		{
			command_success_nodata(si, _("\2**\2 A string containing a chanserv access flag pattern. See: /msg chanserv help flags."));
			command_success_nodata(si, _("\2**\2 If the user has \2any\2 one of the flags specified it is considered a match."));
			break;
		}

		case param::Form::BALLOT:
		{
			command_success_nodata(si, _("\2**\2 A string corresponding to a ballot type generally accepted by the VOTE command."));
			break;
		}
	}

	if(cfg.operlocked(doc,var))
		command_success_nodata(si, _("\2++\2 The network operator has locked this parameter; users may not set it."));
}
catch(const std::exception &e)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_FMTSTR_INTERNAL_ERROR, e.what());
}
catch(...)
{
	command_fail(si, cmd_faultcode_t(0), SPQF_STR_INTERNAL_ERROR);
}


static
void spqf_set(sourceinfo_t *const si,
              const int parc,
              char **const parv)
noexcept try
{
	if(parc < 4)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "SET");
		command_fail(si, fault_needmoreparams, _("Syntax: SET <#channel> <vote type> <category> <parameter> <value>"));
		return;
	}

	const auto &channame(parv[0]);
	auto *const chan(mychan_find(channame));
	if(!chan)
	{
		command_fail(si, fault_nosuch_target, _("Channel is not registered."));
		return;
	}

	const auto uflags(chanacs_source_flags(chan,si));
	if(~uflags & CA_SET && ~uflags & CA_FOUNDER)
	{
		command_fail(si, fault_authfail, _("You are not authorized to use the \2SET\2 command on \2%s\2."),chan->name);
		return;
	}

	const auto &typestr(parv[1]);
	if(!exists<Type>(typestr))
	{
		command_fail(si, fault_badparams, _("This vote type is not valid."));
		return;
	}

	const auto &docstr(parv[2]);
	if(!exists<Doc>(docstr))
	{
		command_fail(si, fault_badparams, _("No such parameter category."));
		return;
	}

	const auto &varstr(parv[3]);
	if(!exists<Var>(varstr))
	{
		command_fail(si, fault_badparams, _("Not a valid parameter name."));
		return;
	}

	const auto type(reflect<Type>(typestr));
	if(type != Type::DEFAULT && !spqf->factory.enabled(type))
	{
		command_fail(si, fault_badparams, _("This vote type is not enabled."));
		return;
	}

	const auto doc(reflect<Doc>(docstr));
	const auto var(reflect<Var>(varstr));
	const auto &value(parc > 4? parv[4] : nullptr);

	char errstr[BUFSIZE] {0};
	if(!param::valid(doc,var,value,errstr,sizeof(errstr)))
	{
		command_fail(si, fault_badparams, "%s", errstr);
		return;
	}

	Cfg cfg(chan,type);
	if(cfg.operlocked(doc,var))
	{
		command_fail(si, fault_authfail, "The network operator has locked this parameter; users may not set it.");
		return;
	}

	cfg.set(doc,var,value);
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


command_t cmd_set
{
	"SET",
	N_(N_("Configure parameters for a channel.")),
	AC_AUTHENTICATED,
	5,
	spqf_set,
	{ nullptr, spqf_set_help }
};


static
void init_doc_tree()
{
	command_t default_initializer
	{
		nullptr,
		nullptr,
		AC_AUTHENTICATED,
		5,
		spqf_set,
		{ nullptr, spqf_set_help }
	};

	for(auto &cmds : doc_cmd)
		for(auto &cmd : cmds)
			memcpy(&cmd,&default_initializer,sizeof(cmd));

	for(size_t i(0); i < param::tree.size(); ++i)
	{
		auto &cmds(doc_cmd[i]);
		auto &tree(doc_tree[i]);
		auto &avail(param::tree[i]);
		for(const auto &var : avail)
		{
			auto &cmd(cmds[uint(get<param::Spec::VAR>(var))]);
			cmd.name = reflect(get<param::Spec::VAR>(var));
			cmd.desc = describe(get<param::Spec::VAR>(var));
			command_add(&cmd, tree);
		}
	}
}


static
void init_docs_tree()
{
	for(size_t i(0); i < num_of<Doc>(); ++i)
	{
		const auto doc(static_cast<Doc>(i));
		command_t cmd
		{
			reflect(doc),
			N_(N_(describe(doc))),
			AC_AUTHENTICATED,
			5,
			spqf_set,
			{ nullptr, spqf_set_help }
		};

		memcpy(&docs_cmd[i],&cmd,sizeof(cmd));
		command_add(&docs_cmd[i], docs_tree);
	}
}


static
void init_types_tree()
{
	for(size_t i(0); i < num_of<Type>(); ++i)
	{
		const auto type(static_cast<Type>(i));
		command_t cmd
		{
			reflect(type),
			N_(N_(describe(type))),
			AC_AUTHENTICATED,
			5,
			spqf_set,
			{ nullptr, spqf_set_help }
		};

		memcpy(&type_cmd[i],&cmd,sizeof(cmd));
		command_add(&type_cmd[i], type_tree);
	}

	// The describe() on type DEFAULT is not good; we want this instead.
	type_cmd[0].desc = N_("Default configuration for all vote types.");
}


void module_init(module_t *const m)
noexcept
{
	module_init_common(m);
	service_bind_command(spqf->svc,&cmd_set);

	docs_tree = mowgli_patricia_create(strcasecanon);
	type_tree = mowgli_patricia_create(strcasecanon);

	for(auto &t : doc_tree)
		t = mowgli_patricia_create(strcasecanon);

	init_types_tree();
	init_docs_tree();
	init_doc_tree();
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	for(auto &t : doc_tree)
		mowgli_patricia_destroy(t, nullptr, nullptr);

	mowgli_patricia_destroy(type_tree, nullptr, nullptr);
	mowgli_patricia_destroy(docs_tree, nullptr, nullptr);

	service_unbind_command(spqf->svc, &cmd_set);
	module_fini_common(intent);
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/set",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
