# SENATVS POPVLVS QVE FREENODVS
###### The Senate & The People of Freenode - Democratic Channel Management System

**SPQF** is a robust **threshold-operator** replacing the need for individual operators to agree, be present, or even exist.
Mimicking the ChanServ interface, **SPQF** requires one or more users to send the same command for effects to occur in a channel.
Which users, their capabilities, and the threshold requirements are all highly configurable.

Example operator #1:

	/msg SPQF quiet #channel nick

Example operator #2:

	/msg SPQF quiet #channel nick

Effects seen in #channel:

	+q *!*@nicks.host


Join **irc.freenode.net/#SPQF** for feedback, suggestions and help.
Every community has different ideas for exactly what they want out of this system, and the complexity of the configuration reflects this;
do not hesitate to consult us if you're planning a deployment in your channel.


### Vote Types Implemented
* **Kick** - Kick users from the channel.
* **Quiet** - Quiet/Unquiet users in the channel.
* **Opine** - Opinion polls that have no effects.


### Getting Started
After inserting the module you must enable a vote type (or DEFAULT for all vote types) in your channel.

	/msg SPQF set #channel quiet vote enable 1

By default this allows unrestricted enfranchisement, or allowing any registered user to send the quiet command.
You can restrict who can use the command by the following:

	/msg SPQF set #channel quiet enfranchise mode v

Now anybody who has voice in the channel can use the command. You can also enfranchise based on the access list:

	/msg SPQF set #channel quiet enfranchise access ov

Now anybody who has chanacs flag +v OR has +o may send the command or vote.
It is also possible to vote against someone using the command like in the original example. Each usage of a command
opens up a new voting motion indexed by an ID number, which is usually announced to the channel (but can be configured
to be silent). After finding the ID number perhaps by using the **/msg SPQF list #channel** command,

	/msg SPQF vote <id> no

This is a vote against the original command, which also counts as one vote itself. The majority wins, so
now a another affirmation is required to break the tie.
