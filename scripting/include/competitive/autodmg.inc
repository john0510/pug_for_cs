static user_dmg[MAX_PLAYERS+1][MAX_PLAYERS+1];
static user_hits[MAX_PLAYERS+1][MAX_PLAYERS+1];

stock dmg_addattack (victim, attacker, damage) {
	user_dmg[victim][attacker] += damage;
	user_hits[victim][attacker]++;
}

stock dmg_reset () {
	for (new i; i < sizeof(user_dmg); i++) {
		arrayset(user_dmg[i], 0, sizeof(user_dmg))
		arrayset(user_hits[i], 0, sizeof(user_hits))
	}
}

stock dmgprint (id, bool:inchat=false) {
	new players[MAX_PLAYERS], count, victim, name[MAX_NAME_LENGTH];
	get_players(players, count, "h");

	for (new i; i<count;i++) {
		victim = players[i];
		
		if (!user_dmg[victim][id])
			continue;

		get_user_name(victim, name, charsmax(name));

		console_print(id, "%L", LANG_SERVER, "PUG_DMG_INFO", name,
					  user_dmg[victim][id], user_hits[victim][id])

		if (inchat)
			chat_print(id, "%L", LANG_SERVER, "PUG_DMG_INFO", name,
						  user_dmg[victim][id], user_hits[victim][id]);
	}

	if (!name[0])
		console_print(id, "%L", LANG_SERVER, "PUG_DMG_DIDNTHURT")
}

stock rdmgprint (id) {
	new players[MAX_PLAYERS], count, attacker, name[MAX_NAME_LENGTH];
	get_players(players, count, "h");
	
	for (new i; i<count;i++) {
		attacker = players[i];
		
		if (!user_dmg[id][attacker])
			continue;

		get_user_name(attacker, name, charsmax(name));

		console_print(id, "%L", LANG_SERVER, "PUG_RDMG_INFO", name,
					  user_dmg[id][attacker], user_hits[id][attacker])
	}
	
	if (!name[0])
		console_print(id, "%L", LANG_SERVER, "PUG_RDMG_DIDNTHURT")
}
