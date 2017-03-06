/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

#include "spqf.h"


static
void handle_drop_chan(mychan_t *const chan)
noexcept try
{
	const auto &cleared(spqf->votes.erase(chan));
	if(cleared > 0)
		slog(LG_INFO,SPQF_NAME": Dropping channel %s erased %zu votes",
		             chan->name,
		             cleared);
}
catch(const std::exception &e)
{
	slog(LG_ERROR,SPQF_NAME": handle_drop_chan unhandled exception: %s",e.what());
}
catch(...)
{
	slog(LG_ERROR,SPQF_NAME": handle_drop_chan unhandled exception");
}


static
void handle_nickchange(hook_user_nick_t *const hook)
noexcept try
{
	spqf->votes.nickchange(hook);
}
catch(const std::exception &e)
{
	slog(LG_ERROR,SPQF_NAME": handle_nickchange unhandled exception: %s",e.what());
}
catch(...)
{
	slog(LG_ERROR,SPQF_NAME": handle_nickchange unhandled exception");
}


static
void write_votes(database_handle_t *const db)
noexcept try
{
	spqf->votes.save(db);
}
catch(const std::exception &e)
{
	slog(LG_ERROR,SPQF_NAME": write_votes unhandled exception: %s",e.what());
}
catch(...)
{
	slog(LG_ERROR,SPQF_NAME": write_votes unhandled exception");
}


static
void read_vote(database_handle_t *const db,
               const char *const type)
noexcept try
{
	spqf->votes.parse(db,type);
}
catch(const std::exception &e)
{
	slog(LG_ERROR,SPQF_NAME": read_vote unhandled exception: %s",e.what());
}
catch(...)
{
	slog(LG_ERROR,SPQF_NAME": read_vote unhandled exception");
}


static
void write_meta(database_handle_t *const db)
noexcept try
{
	const void *md;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(md, &state, metadata->metadata)
	{
		db_start_row(db,SPQF_DBKEY_META);
		db_write_word(db,reinterpret_cast<const metadata_t *>(md)->name);
		db_write_word(db,reinterpret_cast<const metadata_t *>(md)->value);
		db_commit_row(db);
	}
}
catch(const std::exception &e)
{
	slog(LG_ERROR,SPQF_NAME": write_meta unhandled exception: %s",e.what());
}
catch(...)
{
	slog(LG_ERROR,SPQF_NAME": write_meta unhandled exception");
}


static
void read_meta(database_handle_t *const db,
               const char *const type)
noexcept try
{
	const char *const &key(db_sread_word(db));
	const char *const &value(db_sread_str(db));
	metadata_add(metadata,key,value);
}
catch(const std::exception &e)
{
	slog(LG_ERROR,SPQF_NAME": read_meta unhandled exception: %s",e.what());
}
catch(...)
{
	slog(LG_ERROR,SPQF_NAME": read_meta unhandled exception");
}



SPQF::SPQF(module_t *const &m):
svc(service_add(SPQF_NAME,nullptr)),
votes(events,factory),
helpcmd{0,nullptr}
{
	myservice = svc;
	metadata = &mdobj;
	object_init(&mdobj,SPQF_NAME,nullptr);
	hook_add_db_write(write_meta);
	hook_add_db_write(write_votes);
	db_register_type_handler(SPQF_DBKEY_META,&read_meta);
	db_register_type_handler(SPQF_DBKEY_VOTE,&read_vote);
	hook_add_channel_drop(handle_drop_chan);
	hook_add_user_nickchange(handle_nickchange);
}


SPQF::~SPQF()
{
	hook_del_user_nickchange(handle_nickchange);
	hook_del_channel_drop(handle_drop_chan);
	db_unregister_type_handler(SPQF_DBKEY_VOTE);
	db_unregister_type_handler(SPQF_DBKEY_META);
	hook_del_db_write(write_votes);
	hook_del_db_write(write_meta);
	object_dispose(&mdobj);
	service_delete(svc);
}


void module_init(module_t *const m)
noexcept
{
	spqf = new SPQF{m};
}


void module_fini(module_unload_intent_t intent)
noexcept
{
	delete spqf;
	spqf = nullptr;
}


DECLARE_MODULE_V1
(
	SPQF_MODULE"/main",
	MODULE_UNLOAD_CAPABILITY_OK,
	module_init,
	module_fini,
	PACKAGE_STRING,
	"jzk"
);
