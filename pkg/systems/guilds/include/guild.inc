/*
        Maintened by The POL Distro Team

        2009-05-30

        Last Modifications 2010-01-26
             * Fixed PlaceGuildStone()

        Last Modifications 2010-02-04
             * Fixed: Will verify if another guildstone is inside the house. Only 1 GH allowed/house
*/
use uo;
use os;
use util;

include ":security:report";
include "include/housing";

function ViewRosters( mobile, guild, viewer )

	var gump := GFCreateGump();
        showOptionBG( gump, "View Rosters", 1 );

        GFTextLine( gump, 60, 60, 1720, "Player Name" );

        var guild_members := guild.members,
	    num := guild_members.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach member in ( guild_members )

		if( member.serial == guild.GetProp( "Master" ))
			GFTextLine( gump, 60, y_pos, 2100, "GM: "+member.name );
		else
			GFTextLine( gump, 60, y_pos, 2100, member.name );
		endif

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _member_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

        GFSendGump( viewer, gump );

        return showGuildInfo( mobile, viewer );
endfunction

function RecruitAPlayer( mobile, guild, viewer )

	SendSysMessage( viewer, "Who would you like to recruit into the guild?", 3, 88 );
	var targ := Target( viewer );
	if( !targ )
		SendSysMessage( viewer, "Cancelled.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	elseif( targ.serial == mobile.serial )
		SendSysMessage( viewer, "You shall consider therapy.", 3, 44 );
		return showGuildInfo( mobile, viewer );
	elseif( !targ.IsA( POLCLASS_MOBILE ) || targ.IsA( POLCLASS_NPC ))
		SendSysMessage( viewer, "You must target a player.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	elseif( targ.guildid )
		SendSysMessage( viewer, targ.name+" is already in a guild.", 3, 44 );
		return showGuildInfo( mobile, viewer );
	endif

	foreach recruit in ( guild.GetProp( "Recruit" ))
		if( targ.serial == recruit.serial )
			SendSysMessage( viewer, targ.name+" is already recruited for the guild.", 3, 44 );
			return showGuildInfo( mobile, viewer );
		endif
		SleepMS(5);
	endforeach

	SendSysMessage( viewer, "We are waiting the answer...", 3, 68 );

	if( !YesNo( targ, "Would you like to join the guild "+guild.GetProp( "Name" )+"?", "I accept", "I decline", 10 ))
		SendSysMessage( viewer, targ.name+" has declined the invitation.", 3, 44 );
		return showGuildInfo( mobile, viewer );
	else
		SendSysMessage( viewer, targ.name+" has been recruited for the guild.", 3, 68 );
		SendSysMessage( targ, "You have been recruited for the guild.", 3, 68 );

		var recruit := guild.GetProp( "Recruit" );
		if( !recruit )
			recruit := array{};
		endif

		var recruit_list := struct;
		recruit_list.+serial := targ.serial;
		recruit_list.+votefor := 0;
		recruit_list.+voteagainst  := 0;

		var recruit_size := recruit.Size();
		recruit[recruit_size+1] := recruit_list;
		guild.SetProp( "Recruit", recruit );

		return showGuildInfo( mobile, viewer );
	endif
endfunction

function ViewRecruits( mobile, guild, viewer )

	var gump := GFCreateGump();
        showOptionBG( gump, "View Recruits", 1 );

        GFTextLine( gump, 60, 60, 1720, "Player Name" );
        GFTextLine( gump, 260, 60, 1720, "For / Against" );

        var guild_recruits := guild.GetProp( "Recruit" ),
	    num := guild_recruits.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach recruit in ( guild_recruits )

		var the_recruit := SystemFindObjectBySerial( recruit.serial );
		GFTextLine( gump, 60, y_pos, 2100, the_recruit.name );

		GFTextLine( gump, 270, y_pos, 2100, CInt( recruit.votefor ));
		GFTextLine( gump, 325, y_pos, 2100, CInt( recruit.voteagainst ));

		GFAddButton( gump, 40, y_pos+3, 2117, 2118, GF_CLOSE_BTN, _recruit_iter );

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _recruit_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

        var input := GFSendGump( viewer, gump );
        if( !input || input[0xA1] )
			return showGuildInfo( mobile, viewer );
        endif

        input := input[0];

        var player_ref := SystemFindObjectBySerial( guild_recruits[input].serial ),
            vote_for := CInt( guild_recruits[input].votefor ),
            vote_against := CInt( guild_recruits[input].voteagainst );

        if( !Yesno( viewer, "What is your advice for the recruitment of "+player_ref.name+"?", "I vote for "+player_ref.name+".", "I vote against "+player_ref.name+"." ))
			guild_recruits[input].voteagainst := vote_against + 1;
			guild.SetProp( "Recruit", guild_recruits );
        else
			guild_recruits[input].votefor := vote_for + 1;
			guild.SetProp( "Recruit", guild_recruits );
        endif

        return showGuildInfo( mobile, viewer );
endfunction

function ViewAllies( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "View Allies", 1 );

	GFTextLine( gump, 40, 60, 1720, "Guild Name" );
	GFTextLine( gump, 230, 60, 1720, "Started       Days" );

	var guild_allies := guild.AllyGuilds,
	num := guild_allies.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach ally in ( guild_allies )

		GFTextLine( gump, 40, y_pos, 2100, ally.GetProp( "Name" ));

		var peace_delay := guild.GetProp( "PeaceDelay" );
		foreach peace_guild in ( peace_delay )
			if( peace_guild.guildid == ally.guildid )
				GFTextLine( gump, 230, y_pos, 68, StrFormatTime( "%m-%d-%Y", peace_guild.started ));
				GFTextLine( gump, 350, y_pos, 68, peace_guild.days );
				break;
			endif
			SleepMS(5);
		endforeach

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _ally_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

        GFSendGump( viewer, gump );

        return showGuildInfo( mobile, viewer );
endfunction

function ViewEnemies( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "View Ennemies", 1 );

	GFTextLine( gump, 40, 60, 1720, "Guild Name" );
	GFTextLine( gump, 230, 60, 1720, "Started       Days" );

	var guild_enemies := guild.EnemyGuilds,
	num := guild_enemies.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach enemy in ( guild_enemies )

		GFTextLine( gump, 40, y_pos, 2100, enemy.GetProp( "Name" ));

		var war_delay := guild.GetProp( "WarDelay" );
		foreach war_guild in ( war_delay )
			if( war_guild.guildid == enemy.guildid )
				GFTextLine( gump, 230, y_pos, 33, StrFormatTime( "%m-%d-%Y", war_guild.started ));
				GFTextLine( gump, 350, y_pos, 33, war_guild.days );
				break;
			endif
			SleepMS(5);
		endforeach

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _enemy_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

	GFSendGump( viewer, gump );

	return showGuildInfo( mobile, viewer );
endfunction

function PendingStatus( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Guild Status", 1 );

	var total_guild := ListGuilds(),
	num := total_guild.Size();

	GFTextLine( gump, 40, 60, 1720, "Guild Name" );
	GFTextLine( gump, 250, 60, 1720, "Status" );

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach infoGuild in ( total_guild )

		if( infoGuild.guildid == guild.guildid )
			continue;
		endif

		GFTextLine( gump, 40, y_pos, 2100, infoGuild.GetProp( "Name" ));

		if( guild.IsAllyGuild( infoGuild ))
			GFTextLine( gump, 250, y_pos, 68, "Ally" );
		elseif( guild.IsEnemyGuild( infoGuild ))
			GFTextLine( gump, 250, y_pos, 33, "Enemy" );
		else
			GFTextLine( gump, 250, y_pos, 2100, "Neutral" );
		endif

		y_pos += 20;
		counter += 1;

		if( counter >= 17 && _infoGuild_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

	GFSendGump( viewer, gump );

	return showGuildInfo( mobile, viewer );
endfunction

function ToggleTitle( mobile, guild, viewer )

	var abbr := guild.GetProp( "Abbr" ),
		temp_name := "["+abbr+"]";

	if( !GetObjProperty( mobile, "Guild_Abbr" ))
		var guild_title := GetObjProperty( mobile, "Guild_Title" );
		if( !guild_title )
			SendSysMessage( viewer, "You do not have a guild title yet.", 3, 68 );
			mobile.title_guild := abbr;
		else
			mobile.title_guild := guild_title+" "+temp_name;
			SetObjProperty( mobile, "Guild_Abbr", 1 );
		endif
	else
		EraseObjProperty( mobile, "Guild_Abbr" );
		mobile.title_guild := abbr;
	endif

	return showGuildInfo( mobile, viewer );
endfunction

function Resign( mobile, guild, viewer )

	if( mobile.serial == guild.GetProp( "Master" ))
		if( guild.members.Size() > 1 )
			SendSysMessage( viewer, "You must pass the leadership before leaving the guild.", 3, 33 );
			return showGuildInfo( mobile, viewer );
		else
			if( !Yesno( viewer, "Are you sure you want to disband the guild?", "Yes, I am.", "No, do not disband the guild." ))
				return showGuildInfo( mobile, viewer );
			else
				SendSysMessage( mobile, "You are no longer a member of "+guild.GetProp( "Name" )+".", 3, 68 );
				EraseObjProperty( mobile, "Guild_Title" );
				EraseObjProperty( mobile, "Guild_Abbr" );
				EraseObjProperty( mobile, "GuildChat" );
				mobile.title_guild := "";
				guild.RemoveMember( mobile );

				var total_guild := ListGuilds();
				foreach info_guild in ( total_guild )
					if( guild.IsAllyGuild( info_guild ))
						guild.RemoveAllyGuild( info_guild );
					elseif( guild.IsEnemyGuild( info_guild ))
						guild.RemoveEnemyGuild( info_guild );
					endif

					SleepMS(5);
				endforeach

				ReportText( guild.GetProp( "Name" )+" destroyed by "+viewer.name+" Serial: "+viewer.serial, REPORT_SYSLOG );

				guild.EraseProp( "Abbr" );
				guild.EraseProp( "Charter" );
				guild.EraseProp( "GuildIcon" );
				guild.EraseProp( "Master" );
				guild.EraseProp( "Name" );
				guild.EraseProp( "Recruit" );
				guild.EraseProp( "WarDelay" );
				guild.EraseProp( "PeaceDelay" );

				var guildstone_serial := guild.GetProp( "GuildStone" );
				var stone := SystemFindObjectBySerial( guildstone_serial );
				var sign := GetHouseSign( stone );
				EraseObjProperty( sign, "GuildStone" );

				DestroyGuildStone( guild );

				var destroy_guild := DestroyGuild( guild );
				if( destroy_guild.errortext )
					NotifyStaffOnline( viewer, "Error destroying guild -->"+destroy_guild.errortext );
				endif
			endif
		endif
	else
		if( !Yesno( viewer, "Are you sure you want to leave the guild?", "Yes, I am.", "No, I do not leave the guild." ))
			return showGuildInfo( mobile, viewer );
		else
			SendSysMessage( mobile, "You are no longer a member of "+guild.GetProp( "Name" )+".", 3, 68 );
			EraseObjProperty( mobile, "Guild_Title" );
			EraseObjProperty( mobile, "Guild_Abbr" );
			mobile.title_guild := "";
			guild.RemoveMember( mobile );
			ReportText( mobile.name+" resigned from "+guild.GetProp( "Name" )+" by "+viewer.name+" Serial: "+viewer.serial, REPORT_SYSLOG );
		endif
	endif

	return 1;
endfunction

function SetCharter( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Set Charter" );

	GFResizePic( gump, 30, 80, GFGetCfgConst( "Backgrounds", "GOLDTRIM" ), 340, 360 );
	GFTextLine( gump, 30, 440, 1720, "NOTICE: You can use HTML-Commands for your charter." );

	var charter := guild.GetProp( "Charter" ),
	the_msg := array{};

	the_msg.Shrink(0);

	var y_pos := 90;
	var i;
	for( i:=1; i<=18; i+=1 )
		var line := charter[i];
		if ( line == error )
			line := "";
		endif
		the_msg.Append( GFTextEntry( gump, 40, y_pos, 310, 20, 2100, line ));
		y_pos += 20;
	endfor

	var input := GFSendGump( viewer, gump );
	if( !input )
		return showGuildInfo( mobile, viewer );
	else
		var i;
		for( i:=1; i<=the_msg.Size(); i+=1 )
			var line := GFExtractData( input, the_msg[i] );
			if( line )
				the_msg[i] := line;
			else
				the_msg[i] := "";
			endif
		endfor

		//Clear off empty lines.
		for( i:=the_msg.Size(); i>=1; i-=1 )
			if( Len( the_msg[i] ) <= 0 )
				the_msg.Erase(i);
			endif
			SleepMS(2);
		endfor

		if( the_msg.Size() < 1 )
			guild.EraseProp( "Charter" );
		else
			guild.SetProp( "Charter", the_msg );
		endif
	endif

	return showGuildInfo( mobile, viewer );
endfunction

function PlaceGuildStone( mobile )

	var sign := GetHouseSign( mobile );
	if( !sign )
		SendSysMessage( mobile, "You nust be inside a house to place a guild stone.", color := 33 );
		return 0;
	endif
	var guild := FindGuild( mobile.guildid );

	var targ := TargetCoordinates( mobile );
	if( !targ )
		SendSysMessage( mobile, "Cancelled.", 3, 33 );
		return 0;
	elseif( !CheckLosAt( mobile, targ.x, targ.y, targ.z ));
		SendSysMessage( mobile, "Can not see this!", 3, 33 );
		return 0;
	elseif( !sign.IsLocationInsideHouse( targ ) );
		SendSysMessage( mobile, "This is not in a house!", 3, 33 );
		return 0;
	endif

	var guild_house := sign.GetGuildStone();
	var guild_info := guild.GetProp( "GuildStone" );

	if( !sign.IsOwner( mobile ))
		SendSysMessage( mobile, "This is not in your house!", 3, 33 );
		return 0;
	elseif( guild_house && guild_house != guild_info )
		SendSysMessage( mobile, "The house already protect a guildstone!", 3, 33 );
		return 0;
	endif

	if( guild_info )
		DestroyGuildStone( guild );
	endif

	var guild_stone := CreateItemAtLocation( targ.x, targ.y, targ.z, "GuildStone", 1, targ.realm );
	if( guild_stone.errortext )
		SendSysMessage( mobile, "Error creating guild stone -->"+guild_stone.errortext );
		return 0;
	endif

	guild_stone.SetGuild( mobile.guildid );
	SetName( guild_stone, guild.GetProp( "Name" ));
	guild_stone.color := 68;

	guild.SetProp( "GuildStone", guild_stone.serial );
	sign.SetGuildStone( guild_stone.serial );

	return 1;
endfunction

function ChangeGuildName( mobile, guild, viewer )

	var new_name := SendTextEntryGump( viewer, "Select a new name for the guild.", TE_CANCEL_ENABLE, TE_STYLE_NORMAL, CInt( settings.MaxNameLen ));
	if( !new_name )
		SendSysMessage( viewer, "Cancelled.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	elseif( !ValidStr( new_name ))
		SendSysMessage( viewer, "Guild name contains illegal characters.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	elseif( !ValidName( new_name ))
		SendSysMessage( viewer, "Guild name already exists.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	endif

	var new_abbr := SendTextEntryGump( viewer, "Select a new abbreviation for the guild.", TE_CANCEL_ENABLE, TE_STYLE_NORMAL, CInt( settings.MaxAbbrLen ));
	if( !new_abbr )
		SendSysMessage( viewer, "Cancelled.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	elseif( !ValidStr( new_abbr ))
		SendSysMessage( viewer, "Guild abbreviation contains illegal characters.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	elseif( !ValidName( new_abbr ))
		SendSysMessage( viewer, "Guild abbreviation already exists.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	endif

	var regist_fee := CInt( settings.RegistrationFee );
	if( !regist_fee )
		regist_fee := 250000;
	endif

	if( !viewer.SpendGold( regist_fee ) && viewer.cmdlevel < 3 )
		SendSysMessage( viewer, "You do not have enough gold to cover the fee.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	endif

	guild.SetProp( "Name", new_name );
	guild.SetProp( "Abbr", new_abbr );

	foreach member in ( guild.members )
		SetObjProperty( member, "Guild_Abbr", new_abbr );
		member.title_guild := "["+new_abbr+"]";
		EraseObjProperty( member, "Guild_Title" );
		EraseObjProperty( member, "Guild_Abbr" );
		SleepMS(5);
	endforeach

	var guildstone_serial := guild.GetProp( "GuildStone" );
	if( guildstone_serial )
		var guildstone := SystemFindObjectBySerial( guildstone_serial );
		SetName( guildstone, new_name );
	endif

	SendSysMessage( viewer, "Guild name and abbreviation has been changed.", 3, 66 );

	return showGuildInfo( mobile, viewer );
endfunction

function GrantTitle( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Grant Title", 1 );

	GFTextLine( gump, 60, 60, 1720, "Player Name" );
	GFTextLine( gump, 260, 60, 1720, "Title" );

	var guild_members := guild.members,
	num := guild_members.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach member in ( guild_members )

		GFTextLine( gump, 60, y_pos, 2100, member.name );
		GFAddButton( gump, 40, y_pos+3, 2117, 2118, GF_CLOSE_BTN, _member_iter );

		var title := RemoveFromString( member.title_guild, "["+guild.GetProp( "Abbr" )+"]" );
		if( title )
			GFTextLine( gump, 250, y_pos, 2100, title );
		endif

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _member_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

        var input := GFSendGump( viewer, gump );
        if( !input || input[0xA1] )
                return showGuildInfo( mobile, viewer );
        endif

        input := input[0];

        var player_ref := guild_members[input];
        var new_title := SendTextEntryGump( viewer, "What's the new member title?", TE_CANCEL_ENABLE, TE_STYLE_NORMAL, CInt( settings.MaxTitleLen ));
        if( !new_title )
                SendSysMessage( viewer, "Cancelled.", 3, 33 );
                return showGuildInfo( mobile, viewer );
        elseif( !ValidStr( new_title ))
			SendSysMessage( viewer, "Title contains illegal characters.", 3, 33 );
			return showGuildInfo( mobile, viewer );
        endif

        var guild_abbr := "["+guild.GetProp( "Abbr" )+"]";
        SetObjProperty( player_ref, "Guild_Title", new_title );
        SetObjProperty( player_ref, "Guild_Abbr", 1 );

        player_ref.title_guild := new_title+" "+guild_abbr;

        return showGuildInfo( mobile, viewer );
endfunction

function ChangeGuildIcon( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Guild Icon" );

	var list_icons := {0x15a9, 0x15ab, 0x15ad, 0x15af, 0x15b1, 0x15b3, 0x15b5, 0x15b7, 0x15b9, 0x15bb,
		0x15bf, 0x15c1, 0x15c3, 0x15c5, 0x15c7, 0x15c9, 0x15cb, 0x15cd, 0x15cf, 0x15d1 };

	var num_icons := list_icons.Size(),
	guild_icon := guild.GetProp( "GuildIcon" ),
	counter := 0,
	y_pos := 60,
	x_pos := 40;

	GFPage( gump, 1 );
	foreach icon in ( list_icons )

		if( icon == guild_icon )
			GFRadioButton( gump, x_pos, y_pos, icon, icon+1, 1, _icon_iter );
		else
			GFRadioButton( gump, x_pos, y_pos, icon, icon+1, 0, _icon_iter );
		endif

		y_pos += 80;
		counter += 1;

		if( counter >= 5  )
			x_pos += 87;
			y_pos := 60;
			counter := 0;
		endif

		SleepMS(2);
		endforeach

	var input := GFSendGump( viewer, gump );
	if( !input || input[0xA2] )
		return showGuildInfo( mobile, viewer );
	endif

	var new_icon;
	foreach key in ( input.keys )
		if( key >= 1 && key <= num_icons )
			new_icon := list_icons[key];
		endif
		SleepMS(5);
	endforeach

	guild.SetProp( "GuildIcon", new_icon );

	return showGuildInfo( mobile, viewer );
endfunction

function RecruitStatus( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Recruit Status", 1 );

	GFTextLine( gump, 60, 60, 1720, "Player Name" );
	GFTextLine( gump, 260, 60, 1720, "For / Against" );

	var guild_recruits := guild.GetProp( "Recruit" ),
	num := guild_recruits.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach recruit in ( guild_recruits )

		var the_recruit := SystemFindObjectBySerial( recruit.serial );
		GFTextLine( gump, 60, y_pos, 2100, the_recruit.name );

		GFTextLine( gump, 270, y_pos, 2100, CInt( recruit.votefor ));
		GFTextLine( gump, 325, y_pos, 2100, CInt( recruit.voteagainst ));

		GFAddButton( gump, 40, y_pos+3, 2117, 2118, GF_CLOSE_BTN, _recruit_iter );

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _recruit_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

	var input := GFSendGump( viewer, gump );
	if( !input || input[0xA1] )
		return showGuildInfo( mobile, viewer );
	endif

	input := input[0];

	var player_ref := SystemFindObjectBySerial( guild_recruits[input].serial );
	if( !Yesno( viewer, "Are you sure you want to add "+player_ref.name+" to the guild?", "Yes, I am.", "No, I do not accept "+player_ref.name+" in the guild." ))
		SendSysMessage( player_ref, "Your application for "+guild.GetProp( "Name" )+" has been rejected.", 3, 33 );
	else
		foreach member in ( guild.members )
			SendSysMessage( member, player_ref.name+" has been added to the guild.", 3, 66 );
			SleepMS(5);
		endforeach

		SendSysMessage( player_ref, "You are now a member of "+guild.GetProp( "Name" )+". You can use .c to see a list of options.", 3, 88 );

		guild.AddMember( player_ref );
		player_ref.title_guild := "["+guild.GetProp( "Abbr" )+"]";
	endif

	guild_recruits.Erase( input );
	guild.SetProp( "Recruit", guild_recruits );

	return showGuildInfo( mobile, viewer );
endfunction

function DismissMember( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Dismiss Member", 1 );

	GFTextLine( gump, 60, 60, 1720, "Player Name" );

	var guild_members := guild.members,
	num := guild_members.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach member in ( guild_members )

		if( member.serial == mobile.serial )
			continue;
		endif

		GFTextLine( gump, 60, y_pos, 2100, member.name );
		GFAddButton( gump, 40, y_pos+3, 2117, 2118, GF_CLOSE_BTN, _member_iter );

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _member_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

	var input := GFSendGump( viewer, gump );
	if( !input || input[0xA1] )
		return showGuildInfo( mobile, viewer );
	endif

	input := input[0];

	var player_ref := guild_members[input];
	if( Yesno( viewer, "Are you sure you want to dismiss "+player_ref.name+"?", "Yes, I am.", "No, I do not dismiss "+player_ref.name+"." ))
		SendSysMessage( viewer, player_ref.name+" has been removed from the guild.", 3, 68 );
		SendSysMessage( player_ref, "You are no longer member of "+guild.GetProp( "Name" )+".", 3, 68 );
		ReportText( player_ref.name+" resigned from "+guild.GetProp( "Name" )+" by "+viewer.name+" Serial: "+viewer.serial, REPORT_SYSLOG );

		guild.RemoveMember( player_ref );
		player_ref.title_guild := "";
		EraseObjProperty( mobile, "Guild_Title" );
		EraseObjProperty( mobile, "Guild_Abbr" );
		EraseObjProperty( mobile, "GuildChat" );
	endif

	return showGuildInfo( mobile, viewer );
endfunction

function DeclareWar( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Declare War", 1 );

	GFTextLine( gump, 60, 60, 1720, "Guild Name" );

	var total_guild := ListGuilds(),
	num := total_guild.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach infoGuild in ( total_guild )

		if(( infoGuild.IsAllyGuild( guild ) || infoGuild.IsEnemyGuild( guild )) || infoGuild.guildid == guild.guildid )
			continue;
		endif

		GFTextLine( gump, 60, y_pos, 2100, infoGuild.GetProp( "Name" ));
		GFAddButton( gump, 40, y_pos+3, 2117, 2118, GF_CLOSE_BTN, _infoGuild_iter );

		y_pos += 20;
		counter += 1;

		if( counter >= 17 && _infoGuild_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

	var input := GFSendGump( viewer, gump );
	if( !input || input[0xA1] )
		return showGuildInfo( mobile, viewer );
	endif

	input := input[0];

	var guild_ref := total_guild[input];

	if( Yesno( viewer, "Are you sure you want to declare war against "+guild_ref.GetProp( "Name" )+"?", "Yes, I am.", "No, it's a big mistake." ))
		SendSysMessage( viewer, "Your declaration of war is now official!", 3, 68 );

		guild.AddEnemyGuild( guild_ref );
		Broadcast( "The guild "+guild.GetProp( "Name" )+" declares war against "+guild_ref.GetProp( "Name" )+".", 3, 33 );

		var guildWar := guild.GetProp( "WarDelay" );
		if( !guildWar )
			guildWar := array{};
		endif

		var war_list := struct;
		war_list.+guildid := guild_ref.guildid;
		war_list.+started := CInt( POLCore().systime );

		var guildWar_size := guildWar.Size();
		guildWar[guildWar_size+1] := war_list;
		guild.SetProp( "WarDelay", guildWar );

		//We need to store these infos to the other guild too...
		var guild_refWar := guild_ref.GetProp( "WarDelay" );
		if( !guild_refWar )
			guild_refWar := array{};
		endif

		war_list.guildid := guild.guildid;

		var guild_refWar_size := guild_refWar.Size();
		guild_refWar[guild_refWar_size+1] := war_list;
		guild_ref.SetProp( "WarDelay", guild_refWar );
	endif

	return showGuildInfo( mobile, viewer );
endfunction

function DeclarePeace( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Declare Peace", 1 );

	GFTextLine( gump, 60, 60, 1720, "Guild Name" );

	var total_guild := ListGuilds(),
	num := total_guild.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach infoGuild in ( total_guild )

		if(( infoGuild.IsAllyGuild( guild ) || infoGuild.IsEnemyGuild( guild )) || infoGuild.guildid == guild.guildid )
			continue;
		endif

		GFTextLine( gump, 60, y_pos, 2100, infoGuild.GetProp( "Name" ));
		GFAddButton( gump, 40, y_pos+3, 2117, 2118, GF_CLOSE_BTN, _infoGuild_iter );

		y_pos += 20;
		counter += 1;

		if( counter >= 17 && _infoGuild_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

	var input := GFSendGump( viewer, gump );
	if( !input || input[0xA1] )
		return showGuildInfo( mobile, viewer );
	endif

	input := input[0];

	var guild_ref := total_guild[input];
	var days_peace := SendTextEntryGump( viewer, "How long shall be the alliance? [1-"+CInt( settings.MaxWarPeaceStatutDays )+" days]", TE_CANCEL_ENABLE, TE_STYLE_NUMERICAL, CInt( settings.MaxWarPeaceStatutDays ));
	if( !CInt( days_peace ))
		SendSysMessage( viewer, "Cancelled.", 3, 33 );
		return showGuildInfo( mobile, viewer );
	endif

	if( Yesno( viewer, "Are you sure you want to declare an alliance with "+guild_ref.GetProp( "Name" )+"?", "Yes, I am.", "No, I do not declare an alliance with "+guild_ref.GetProp( "Name" )+"." ))
		SendSysMessage( viewer, "Your alliance has been made.", 3, 66 );

		guild.AddAllyGuild( guild_ref );
		Broadcast( "The guild "+guild.GetProp( "Name" )+" declares an alliance with "+guild_ref.GetProp( "Name" )+" for "+days_peace+" days.", 3, 68 );

		var guildPeace := guild.GetProp( "PeaceDelay" );
		if( !guildPeace )
			guildPeace := array{};
		endif

		var peace_list := struct;
		peace_list.+guildid := guild_ref.guildid;
		peace_list.+started := CInt( POLCore().systime );
		peace_list.+days  := days_peace;

		var guildPeace_size := guildPeace.Size();
		guildPeace[guildPeace_size+1] := peace_list;
		guild.SetProp( "PeaceDelay", guildPeace );

		//We need to add these info to the other guild too...
		var guild_refPeace := guild_ref.GetProp( "PeaceDelay" );
		if( !guild_refPeace )
			guild_refPeace := array{};
		endif

		peace_list.guildid := guild.guildid;

		var guild_refPeace_size := guild_refPeace.Size();
		guild_refPeace[guild_refPeace_size+1] := peace_list;
		guild_ref.SetProp( "PeaceDelay", guild_refPeace );
	endif

	return showGuildInfo( mobile, viewer );
endfunction

function TransferLeader( mobile, guild, viewer )

	var gump := GFCreateGump();
	showOptionBG( gump, "Transfer Leadership", 1 );

	GFTextLine( gump, 60, 60, 1720, "Player Name" );

	var guild_members := guild.members,
	num := guild_members.Size();

	var counter := 0;
	var y_pos := 100;

	GFPage( gump, 1 );
	foreach member in ( guild_members )

		if( member.serial == mobile.serial )
			continue;
		endif

		GFTextLine( gump, 60, y_pos, 2100, member.name );
		GFAddButton( gump, 40, y_pos+3, 2117, 2118, GF_CLOSE_BTN, _member_iter );

		y_pos += 20;
		counter += 1;

		if ( counter >= 17 && _member_iter < num )
			GFAddButton( gump, 20, 400, 0xA92, 0xA93, GF_PAGE_BTN, gump.cur_page+1 );
			GFPage( gump, gump.cur_page+1 );
			GFAddButton( gump, 20, 80, 0xA90, 0xA91, GF_PAGE_BTN, gump.cur_page-1 );
			counter := 0;
			y_pos := 80;
		endif
		SleepMS(2);
	endforeach

	var input := GFSendGump( viewer, gump );
	if( !input || input[0xA1] )
		return showGuildInfo( mobile, viewer );
	endif

	input := input[0];

	var player_ref := guild_members[input];
	if( Yesno( viewer, "Are you sure you want to transfer leadership to "+player_ref.name+"?", "Yes, I am.", "No, I do not transfer the leadership." ))
		SendSysMessage( viewer, player_ref.name+" is now the Guild Master.", 3, 68 );
		SendSysMessage( player_ref, "You are now the Guild Master of "+guild.GetProp( "Name" )+".", 3, 68 );
		guild.SetProp( "Master", player_ref.serial );
		DestroyGuildStone( guild );
		toggle_menu := 0;
	endif

	return showGuildInfo( mobile, viewer );
endfunction

function showOptionBG( byref gump, menu, no_cancel:=0 )

	GFResizePic( gump, 0, 0, GFGetCfgConst( "Defaults", "BackGround" ), 400, 520 );
	GFResizePic( gump, 15, 15, GFGetCfgConst( "Defaults", "ForeGround" ), 370, 25 );
	GFResizePic( gump, 15, 45, GFGetCfgConst( "Defaults", "ForeGround" ), 370, 465 );
	GFTextMid( gump, 15, 17, 400, 1720, menu );

	if( !no_cancel )
		GFAddButton( gump, 100, 470, 2119, 2120, GF_CLOSE_BTN, 0xA2 ); // Cancel
	endif

	GFAddButton( gump, 25, 470, 2128, 2129, GF_CLOSE_BTN, 0xA1 ); // Okay

	return gump;
endfunction

function DestroyGuildStone( guild )

	var guildstone_serial := guild.GetProp( "GuildStone" );
	if( guildstone_serial )
		DestroyItem( SystemFindObjectBySerial( guildstone_serial ));
	endif

	return 1;
endfunction
