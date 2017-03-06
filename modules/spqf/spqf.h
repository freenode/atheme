/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

extern "C"
{
	#include "atheme.h"
}

#include <stdexcept>
#include <exception>
#include <functional>
#include <string>
#include <memory>
#include <algorithm>
#include <deque>
#include <set>
#include <map>

using std::begin;
using std::end;
using std::get;


// Service name                      (note: switch to VoteServ here)
#define SPQF_NAME                    "SPQF"
#define SPQF_MODULE                  "spqf"
#define SPQF_TITLE                   "The Senate & The People Of freenode"

// Database key for votes
#define SPQF_DBKEY_VOTE              SPQF_NAME "V"

// Database key for unassociated metadata
#define SPQF_DBKEY_META              SPQF_NAME "M"

// Metadata prefixes
#define SPQF_META_CHAN_NS            "private:" SPQF_NAME ":"
#define SPQF_META_CFG_NS             "c:"

// Misc
#define SPQF_STR_INTERNAL_ERROR      SPQF_NAME ": An internal error has occurred. Please contact the developer."
#define SPQF_FMTSTR_INTERNAL_ERROR   SPQF_NAME ": An internal error has occurred. Please contact the developer and include: [%s]"


/* Convenience initialized by common module init.
 */
service_t *myservice;
object_t *metadata;


namespace colors
{
	#include "colors.h"
}
#include "util.h"
#include "Metadata.h"
#include "Type.h"
#include "State.h"
#include "Doc.h"
#include "Var.h"
#include "Cfg.h"
#include "Ballots.h"
#include "Vote.h"
#include "Factory.h"
#include "Events.h"
#include "Votes.h"


/* By setting this state (in class SPQF) from the main help handler (help.c)
 * subcommand help handlers may have an easier time with complex command syntaxes.
 */
using HelpCmd = std::pair<int,char**>;


/* Main object (singleton, from main.c)
 */
struct SPQF
{
	service_t *svc;
	object_t mdobj;
	Events events;
	Factory factory;
	Votes votes;
	HelpCmd helpcmd;

	SPQF(module_t *const &m);
	~SPQF();
};


/* Pointer to the main SPQF instance.
 * Initialized by the module_init_common() (so make sure you call it!)
 * This is the only symbol that is shared between modules.
 */
SPQF *spqf;


/* Convenience to check if a module with a certain vote type is active
 */
inline
bool enabled(const Type &type)
{
	return spqf->factory.enabled(type);
}


/* Must be called by additional modules.
 * If the module specifies a vote type call the overload below instead.
 */
inline
void module_init_common(module_t *const &m)
{
	void *sym_ptr;
	MODULE_TRY_REQUEST_SYMBOL(m, sym_ptr, SPQF_MODULE"/main", SPQF_MODULE);
	auto dp(reinterpret_cast<void **>(sym_ptr));
	spqf = reinterpret_cast<SPQF *>(*dp);
	myservice = spqf->svc;
	metadata = &spqf->mdobj;
}


/* Must be called by additional modules that specify a vote type.
 */
template<class TypeCallback>
void module_init_common(module_t *const &m,
                        const Type &type,
                        TypeCallback&& callback)
{
	module_init_common(m);
	spqf->factory.set_callback(type,std::forward<TypeCallback>(callback));
}


/* Must be called by additional modules when destructing.
 * If the module specifies a vote type call the overload below instead.
 */
inline
void module_fini_common(module_unload_intent_t intent)
{
}


/* Must be called by additional modules that specify a vote type, when destructing.
 */
inline
void module_fini_common(module_unload_intent_t intent,
                        const Type &type)
{
	if(spqf)
	{
		spqf->votes.erase(type);
		spqf->factory.unset_callback(type);
	}

	module_fini_common(intent);
}
