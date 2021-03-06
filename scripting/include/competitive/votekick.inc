#if defined _votekick_included
  #endinput
#endif

#define _votekick_included

#define TASK_HUD_VOTEKICK 7001

new const MAX_VOTEKICKS = 3;

static target[TeamName], target_name[TeamName][32];
static votekick_limiter[33];

static votes[TeamName][2];
static teammates_count[TeamName];
static bool:is_voting[TeamName];
static players_kicked[10][32];

static vote_delay;
static vote_enable;

stock votekick_init () {
	vote_enable = register_cvar("pug_votekick_enable", "1");
	vote_delay = register_cvar("pug_votekick_delay", "10.0");
}

stock is_votekick_enable ()
	return get_pcvar_num(vote_enable) ? true : false;

stock votekick_menucreate (name[]) {
	static title[100];
	formatex(title, sizeof(title) -1, "%L", LANG_SERVER,
	         "PUG_VOTEKICK_TITLE", name);

	new menu = menu_create(title, "votekick_menuhandler");

	new option_yes[5], option_no[5];
	format(option_yes, charsmax(option_yes), "%L", LANG_SERVER, "YES");
	format(option_no, charsmax(option_no), "%L", LANG_SERVER, "NO");

	menu_additem(menu, option_yes);
	menu_additem(menu, option_no);

	menu_setprop(menu, MPROP_EXIT, MEXIT_NEVER);

	return menu;
}

public votekick_menuhandler (const id, menu, item) {
	if (item == MENU_EXIT)
		return PLUGIN_HANDLED;
	
	new const TeamName:team = client_get_team(id);

	votes[team][item]++;
	votekick_list(team);

	if (votekick_isready(team))
		votekick_finish(team, menu);

	return PLUGIN_HANDLED;
}

stock votekick_start (id, target_id) {
	new TeamName:team;
	new name[32];

	team = client_get_team(id);
	get_user_name(id, name, charsmax(name));

	get_user_name(target_id, target_name[team], charsmax(target_name[]));

	if (!votekick_validate(id, target_id, team))
		return;

	target[team] = target_id;
	is_voting[team] = true;
	votekick_limiter[id]++;
	arrayset(votes[team], false, sizeof(votes));

	new menu = votekick_menucreate(target_name[team]);
	teammates_count[team] = menu_display_team(team, menu);

	new args[2];
	args[0] = _:team;
	args[1] = menu;
	set_task(get_votekick_delay(), "votekick_finish_task", _, args, sizeof(args), "a", 1);
	set_task(0.1, "votekick_list_task", TASK_HUD_VOTEKICK, args, sizeof(args), "b")

	team_print(team, "%L para votekick", LANG_SERVER, "PUG_VOTING_STARTED", name);
}

public votekick_finish_task (args[]) {
	if (is_voting[TeamName:args[0]])
		votekick_finish(TeamName:args[0], args[1]);
}

public votekick_list_task (args[])
	votekick_list(TeamName:args[0]);

stock votekick_list (TeamName:team) {
	new hud[512];
	new votes_yes, votes_no;

	votes_yes = votes[team][0];
	votes_no = votes[team][1];

	if (votes_yes)
		format(hud, charsmax(hud), "%s[%i] %L^n", hud, votes_yes, LANG_SERVER, "YES")
	if (votes_no)
		format(hud, charsmax(hud), "%s[%i] %L^n", hud, votes_no, LANG_SERVER, "NO")

	if (!(votes_yes + votes_no)) 
		formatex(hud, charsmax(hud), "%L", LANG_SERVER, "PUG_NOVOTES")

	showt_hudtitle(team, "Votekick");
	showt_hudbody(team, hud);
}

stock votekick_finish (TeamName:team, menu) {
	is_voting[team] = false;

	remove_task(TASK_HUD_VOTEKICK)
	menu_destroy(menu);
	menu_cancel_team(team);

	if (votes[team][0] < teammates_count[team] - 1 || !votes[team][0]) {
		team_print(team, "%L", LANG_SERVER, "PUG_VOTING_INSUFFICIENT");
		return;
	}

	user_kick(target[team]);
	
	chat_print(0, "%L", LANG_SERVER, "PUG_VOTEKICK_KICKED", target_name[team]);
}

stock user_iskicked (id) {
	static steamid[32];

	for (new i; i<sizeof players_kicked; i++) {
		get_user_authid(id, steamid, sizeof(steamid) -1);

		if (equali(players_kicked[i], steamid))
			return true;
	}

	return false;
}

static user_kick (id) {
	static steamid[32];
	get_user_authid(id, steamid, sizeof(steamid) -1);

	for (new i; i<sizeof players_kicked; i++) {
		if (!equali(players_kicked[i], ""))
			continue;

		server_cmd("kick #%i ^"Votekick^"", get_user_userid(id));

		format(players_kicked[i], charsmax(players_kicked[]), "%s", steamid);
		break;
	}
}

stock votekick_reset () {
	votekick_unban();
	arrayset(votekick_limiter, 0, sizeof(votekick_limiter));
}

stock votekick_unban () {
	for (new i; i<sizeof players_kicked; i++)
		format(players_kicked[i], sizeof(players_kicked[]), "");
}

static Float:get_votekick_delay ()
	return get_pcvar_float(vote_delay);

stock bool:votekick_validate (id, target_id, TeamName:team) {
	new TeamName:target_team = client_get_team(target_id);

	if (!team_can_vote(team) ||
		is_voting[team] ||
	    team != target_team)
	{
		return false;
	}

	if (votekick_limiter[id] >= MAX_VOTEKICKS) {
		chat_print(id, "%L", LANG_SERVER, "PUG_VOTEKICK_MAXVOTES");
		return false;
	}

	return true;
}

stock votekick_totalvotes (TeamName:team)
	return votes[team][0] + votes[team][1];

stock bool:votekick_isready (TeamName:team) {
	new const total_votes = votekick_totalvotes(team);

	if (total_votes >= teammates_count[team])
		return true;

	return false;
}

