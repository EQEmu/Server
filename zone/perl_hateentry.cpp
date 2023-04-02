#include "../common/features.h"
#include "client.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"
#include "hate_list.h"

Mob* Perl_HateEntry_GetEnt(struct_HateList* self) // @categories Script Utility, Hate and Aggro
{
	return self->entity_on_hatelist;
}

int64_t Perl_HateEntry_GetHate(struct_HateList* self) // @categories Script Utility, Hate and Aggro
{
	return self->stored_hate_amount;
}

int64_t Perl_HateEntry_GetDamage(struct_HateList* self) // @categories Script Utility, Hate and Aggro
{
	return self->hatelist_damage;
}

void perl_register_hateentry()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<struct_HateList>("HateEntry");
	package.add("GetDamage", &Perl_HateEntry_GetDamage);
	package.add("GetEnt", &Perl_HateEntry_GetEnt);
	package.add("GetHate", &Perl_HateEntry_GetHate);
}

#endif //EMBPERL_XS_CLASSES

