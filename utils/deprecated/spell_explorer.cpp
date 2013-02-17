#include <stdio.h>
#include "../common/types.h"
#include "../common/seperator.h"
#include "../zone/spdat.h"

int main(int argc, char** argv) {
	
	int spid = 0;
	
	if(argc != 2) {
		printf("Invalid args: %s [spell id]\n", argv[0]);
		return(1);
	}
	
	spid = atoi(argv[1]);
	
	
	int tempid=0;
	int16 counter=0;
	char spell_line[2048];
	
	FILE *sf = fopen("spells_us.txt", "r");
	
	if(sf == NULL) {
		printf("Unable to open spells_us.txt file.\n");
		return false;
	}
	
	SPDat_Spell_Struct sp;
		
	while(!feof(sf)) {
		fgets(spell_line, sizeof(spell_line), sf);
		Seperator sep(spell_line, '^', 205, 100, false, 0, 0, false);
		
		if(spell_line[0]=='\0')
			break;
		
		tempid = atoi(sep.arg[0]);
		if(tempid != spid)
			continue;
		
		printf("Found spell %d\n", spid);
		
		counter++;
		strcpy(sp.name, sep.arg[1]);
		strcpy(sp.player_1, sep.arg[2]);
		strcpy(sp.teleport_zone, sep.arg[3]);
		strcpy(sp.you_cast,  sep.arg[4]);
		strcpy(sp.other_casts, sep.arg[5]);
		strcpy(sp.cast_on_you, sep.arg[6]);
		strcpy(sp.cast_on_other, sep.arg[7]);
		strcpy(sp.spell_fades, sep.arg[8]);

		sp.range=atof(sep.arg[9]);
		sp.aoerange=atof(sep.arg[10]);
		sp.pushback=atof(sep.arg[11]);
		sp.pushup=atof(sep.arg[12]);
		sp.cast_time=atoi(sep.arg[13]);
		sp.recovery_time=atoi(sep.arg[14]);
		sp.recast_time=atoi(sep.arg[15]);
		sp.buffdurationformula=atoi(sep.arg[16]);
		sp.buffduration=atoi(sep.arg[17]);
		sp.AEDuration=atoi(sep.arg[18]);
		sp.mana=atoi(sep.arg[19]);
		
		int y=0;
		for(y=0; y < EFFECT_COUNT;y++)
			sp.base[y]=atoi(sep.arg[20+y]);
		for(y=0;y<11;y++)
			sp.base2[y]=atoi(sep.arg[33+y]);
		for(y=0; y < EFFECT_COUNT;y++)
			sp.max[y]=atoi(sep.arg[44+y]);
		
		sp.icon=atoi(sep.arg[56]);
		sp.memicon=atoi(sep.arg[57]);
		
		for(y=0; y< 4;y++)
			sp.components[y]=atoi(sep.arg[58+y]);
		
		for(y=0; y< 4;y++)
			sp.component_counts[y]=atoi(sep.arg[62+y]);
		
		for(y=0; y< 4;y++)
			sp.NoexpendReagent[y]=atoi(sep.arg[66+y]);
		
		for(y=0; y< 12;y++)
			sp.formula[y]=atoi(sep.arg[70+y]);
		
		sp.LightType=atoi(sep.arg[82]);
		sp.goodEffect=atoi(sep.arg[83]);
		sp.Activated=atoi(sep.arg[84]);
		sp.resisttype=atoi(sep.arg[85]);
		
		for(y=0; y< 12;y++)
			sp.effectid[y]=atoi(sep.arg[86+y]);
		
		sp.targettype=(SpellTargetType)atoi(sep.arg[98]);
		sp.basediff=atoi(sep.arg[99]);
		sp.skill=(SkillType)atoi(sep.arg[100]);
		sp.zonetype=atoi(sep.arg[101]);
		sp.EnvironmentType=atoi(sep.arg[102]);
		sp.TimeOfDay=atoi(sep.arg[103]);
		
		for(y=0; y< 16;y++)
			sp.classes[y]=atoi(sep.arg[104+y]);
		
		sp.CastingAnim=atoi(sep.arg[120]);
		sp.TargetAnim=atoi(sep.arg[121]);
		sp.TravelType=atoi(sep.arg[122]);
		sp.SpellAffectIndex=atoi(sep.arg[123]);
		
		for(y=0; y< 23;y++) {
			sp.spacing124[y]=atoi(sep.arg[124+y]);
		}
		
		sp.ResistDiff=atoi(sep.arg[147]);
		sp.dot_stacking_exempt=atoi(sep.arg[148]);
		sp.deletable=atoi(sep.arg[149]);
		
		sp.RecourseLink = atoi(sep.arg[150]);
		sp.descnum = atoi(sep.arg[155]);
		sp.typedescnum = atoi(sep.arg[156]);
		sp.effectdescnum = atoi(sep.arg[157]);

//		for(y=0; y< 17;y++)
//			sp.Spacing4[y] = atoi(sep.arg[158+y]);
		
		break;
	}

	fclose(sf);
	
	const struct SPDat_Spell_Struct *s=&sp;
	
	printf("Spell info for spell #%d:\n", spid);
	printf("  name: %s\n", s->name);
	printf("  player_1: %s\n", s->player_1);
	printf("  teleport_zone: %s\n", s->teleport_zone);
	printf("  you_cast: %s\n", s->you_cast);
	printf("  other_casts: %s\n", s->other_casts);
	printf("  cast_on_you: %s\n", s->cast_on_you);
	printf("  spell_fades: %s\n", s->spell_fades);
	printf("  range: %f\n", s->range);
	printf("  aoerange: %f\n", s->aoerange);
	printf("  pushback: %f\n", s->pushback);
	printf("  pushup: %f\n", s->pushup);
	printf("  cast_time: %d\n", s->cast_time);
	printf("  recovery_time: %d\n", s->recovery_time);
	printf("  recast_time: %d\n", s->recast_time);
	printf("  buffdurationformula: %d\n", s->buffdurationformula);
	printf("  buffduration: %d\n", s->buffduration);
	printf("  AEDuration: %d\n", s->AEDuration);
	printf("  mana: %d\n", s->mana);
	printf("  base[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", s->base[0], s->base[1], s->base[2], s->base[3], s->base[4], s->base[5], s->base[6], s->base[7], s->base[8], s->base[9], s->base[10], s->base[11]);
	printf("  base2[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", s->base2[0], s->base2[1], s->base2[2], s->base2[3], s->base2[4], s->base2[5], s->base2[6], s->base2[7], s->base2[8], s->base2[9], s->base2[10], s->base2[11]);
	printf("  max[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", s->max[0], s->max[1], s->max[2], s->max[3], s->max[4], s->max[5], s->max[6], s->max[7], s->max[8], s->max[9], s->max[10], s->max[11]);
	printf("  icon: %d\n", s->icon);
	printf("  memicon: %d\n", s->memicon);
	printf("  components[4]: %d, %d, %d, %d\n", s->components[0], s->components[1], s->components[2], s->components[3]);
	printf("  component_counts[4]: %d, %d, %d, %d\n", s->component_counts[0], s->component_counts[1], s->component_counts[2], s->component_counts[3]);
	printf("  NoexpendReagent[4]: %d, %d, %d, %d\n", s->NoexpendReagent[0], s->NoexpendReagent[1], s->NoexpendReagent[2], s->NoexpendReagent[3]);
	printf("  formula[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", s->formula[0], s->formula[1], s->formula[2], s->formula[3], s->formula[4], s->formula[5], s->formula[6], s->formula[7], s->formula[8], s->formula[9], s->formula[10], s->formula[11]);
	printf("  LightType: %d\n", s->LightType);
	printf("  goodEffect: %d\n", s->goodEffect);
	printf("  Activated: %d\n", s->Activated);
	printf("  resisttype: %d\n", s->resisttype);
	printf("  effectid[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", s->effectid[0], s->effectid[1], s->effectid[2], s->effectid[3], s->effectid[4], s->effectid[5], s->effectid[6], s->effectid[7], s->effectid[8], s->effectid[9], s->effectid[10], s->effectid[11]);
	printf("  targettype: %d\n", s->targettype);
	printf("  basediff: %d\n", s->basediff);
	printf("  skill: %d\n", s->skill);
	printf("  zonetype: %d\n", s->zonetype);
	printf("  EnvironmentType: %d\n", s->EnvironmentType);
	printf("  TimeOfDay: %d\n", s->TimeOfDay);
	printf("  classes[15]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			s->classes[0], s->classes[1], s->classes[2], s->classes[3], s->classes[4],
			s->classes[5], s->classes[6], s->classes[7], s->classes[8], s->classes[9],
			s->classes[10], s->classes[11], s->classes[12], s->classes[13], s->classes[14]);
	printf("  CastingAnim: %d\n", s->CastingAnim);
	printf("  TargetAnim: %d\n", s->TargetAnim);
	printf("  SpellAffectIndex: %d\n", s->SpellAffectIndex);
	printf(" RecourseLink: %d\n", s->RecourseLink);
	printf("  Spacing124[23]: %d, %d, %d, %d, %d\n", s->spacing124[0], s->spacing124[1], s->spacing124[2], s->spacing124[3], s->spacing124[4]);

	

	return(0);
}

