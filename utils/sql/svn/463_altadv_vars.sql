#
# The spell_type field in the altadv_vars table is used by the client as a timer ID and is
# sent in OP_AAAction timer update packets.
#
# AAs on a shared timer should have the same spell_type. 
#
# Clean out any existing timers using the previous method of numbering AA Timers.
#
delete from timers where type>=1000 and type<=2999;
#
# This updates the spell_type (Timer ID) for all existing entries that
# have a reuse timer.
#
# Most of the numbers have been collected from live, with a few tweaks where AAs have been removed/combined.
#
update altadv_vars set spell_type=1 where skill_id =128;
update altadv_vars set spell_type=2 where skill_id =129;
update altadv_vars set spell_type=3 where skill_id =130;
update altadv_vars set spell_type=4 where skill_id =131;
update altadv_vars set spell_type=5 where skill_id =132;
update altadv_vars set spell_type=6 where skill_id =133;
update altadv_vars set spell_type=7 where skill_id =136;
update altadv_vars set spell_type=3 where skill_id =140;
update altadv_vars set spell_type=15 where skill_id =145;
update altadv_vars set spell_type=2 where skill_id =146;
update altadv_vars set spell_type=3 where skill_id =153;
update altadv_vars set spell_type=7 where skill_id =154;
update altadv_vars set spell_type=8 where skill_id =155;
update altadv_vars set spell_type=2 where skill_id =156;
update altadv_vars set spell_type=3 where skill_id =162;
update altadv_vars set spell_type=2 where skill_id =163;
update altadv_vars set spell_type=14 where skill_id =167;
update altadv_vars set spell_type=4 where skill_id =168;
update altadv_vars set spell_type=4 where skill_id =171;
update altadv_vars set spell_type=4 where skill_id =174;
update altadv_vars set spell_type=4 where skill_id =177;
update altadv_vars set spell_type=5 where skill_id =181;
update altadv_vars set spell_type=4 where skill_id =183;
update altadv_vars set spell_type=5 where skill_id =184;
update altadv_vars set spell_type=6 where skill_id =185;
update altadv_vars set spell_type=7 where skill_id =186;
update altadv_vars set spell_type=8 where skill_id =187;
update altadv_vars set spell_type=9 where skill_id =188;
update altadv_vars set spell_type=3 where skill_id =193;
update altadv_vars set spell_type=0 where skill_id =194;
update altadv_vars set spell_type=2 where skill_id =197;
update altadv_vars set spell_type=0 where skill_id =206;
update altadv_vars set spell_type=2 where skill_id =207;
update altadv_vars set spell_type=2 where skill_id =208;
update altadv_vars set spell_type=1 where skill_id =233;
update altadv_vars set spell_type=1 where skill_id =243;
update altadv_vars set spell_type=12 where skill_id =254;
update altadv_vars set spell_type=1 where skill_id =258;
update altadv_vars set spell_type=2 where skill_id =259;
update altadv_vars set spell_type=3 where skill_id =260;
update altadv_vars set spell_type=4 where skill_id =274;
update altadv_vars set spell_type=3 where skill_id =289;
update altadv_vars set spell_type=4 where skill_id =290;
update altadv_vars set spell_type=5 where skill_id =291;
update altadv_vars set spell_type=8 where skill_id =459;
update altadv_vars set spell_type=9 where skill_id =507;
update altadv_vars set spell_type=4 where skill_id =510;
update altadv_vars set spell_type=4 where skill_id =513;
update altadv_vars set spell_type=5 where skill_id =516;
update altadv_vars set spell_type=1 where skill_id =517;
update altadv_vars set spell_type=6 where skill_id =520;
update altadv_vars set spell_type=9 where skill_id =523;
update altadv_vars set spell_type=0 where skill_id =526;
update altadv_vars set spell_type=5 where skill_id =528;
update altadv_vars set spell_type=4 where skill_id =531;
update altadv_vars set spell_type=8 where skill_id =533;
update altadv_vars set spell_type=4 where skill_id =534;
update altadv_vars set spell_type=3 where skill_id =545;
update altadv_vars set spell_type=5 where skill_id =548;
update altadv_vars set spell_type=2 where skill_id =592;
update altadv_vars set spell_type=7 where skill_id =616;
update altadv_vars set spell_type=6 where skill_id =619;
update altadv_vars set spell_type=7 where skill_id =630;
update altadv_vars set spell_type=0 where skill_id =643;
update altadv_vars set spell_type=4 where skill_id =645;
update altadv_vars set spell_type=7 where skill_id =718;
update altadv_vars set spell_type=6 where skill_id =721;
update altadv_vars set spell_type=6 where skill_id =723;
update altadv_vars set spell_type=8 where skill_id =921;
update altadv_vars set spell_type=8 where skill_id =922;
update altadv_vars set spell_type=8 where skill_id =923;
update altadv_vars set spell_type=12 where skill_id =926;
update altadv_vars set spell_type=13 where skill_id =931;
update altadv_vars set spell_type=20 where skill_id =1000;
update altadv_vars set spell_type=8 where skill_id =1119;
update altadv_vars set spell_type=5 where skill_id =1123;
update altadv_vars set spell_type=2 where skill_id =1126;
update altadv_vars set spell_type=10 where skill_id =1229;
update altadv_vars set spell_type=11 where skill_id =1334;
update altadv_vars set spell_type=13 where skill_id =1337;
update altadv_vars set spell_type=8 where skill_id =1340;
update altadv_vars set spell_type=8 where skill_id =1341;
update altadv_vars set spell_type=8 where skill_id =1342;
update altadv_vars set spell_type=9 where skill_id =1343;
update altadv_vars set spell_type=8 where skill_id =1344;
update altadv_vars set spell_type=2 where skill_id =828;
update altadv_vars set spell_type=10 where skill_id =831;
update altadv_vars set spell_type=5 where skill_id =872;
update altadv_vars set spell_type=5 where skill_id =875;
update altadv_vars set spell_type=5 where skill_id =860;
update altadv_vars set spell_type=6 where skill_id =863;
update altadv_vars set spell_type=5 where skill_id =900;
update altadv_vars set spell_type=10 where skill_id =746;
update altadv_vars set spell_type=13 where skill_id =749;
update altadv_vars set spell_type=6 where skill_id =757;
update altadv_vars set spell_type=5 where skill_id =1149;
update altadv_vars set spell_type=2 where skill_id =1150;
update altadv_vars set spell_type=5 where skill_id =773;
update altadv_vars set spell_type=8 where skill_id =785;
update altadv_vars set spell_type=2 where skill_id =702;
update altadv_vars set spell_type=12 where skill_id =209;
