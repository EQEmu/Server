#include "global_define.h"
#include "types.h"
#include "clientversions.h"
#include "eq_packet.h"
#include "file_verify_manager.h"
#include "string_util.h"
#include <memory>

struct EQEmu::FileVerifyManager::impl {
	std::unique_ptr<FileVerify> spell_data;
	std::unique_ptr<FileVerify> skill_data;
	std::unique_ptr<FileVerify> base_data;
	std::unique_ptr<FileVerify> eqgames[(int)ClientVersion::MaxClientVersions];
};

EQEmu::FileVerifyManager::FileVerifyManager() {
	impl_ = new impl;
	impl_->spell_data.reset(new FileVerify());
	impl_->spell_data->Load("verify/spells_us.txt");

	impl_->skill_data.reset(new FileVerify());
	impl_->skill_data->Load("verify/SkillCaps.txt");

	impl_->base_data.reset(new FileVerify());
	impl_->base_data->Load("verify/BaseData.txt");

	for(int i = 0; i < (int)ClientVersion::MaxClientVersions; ++i) {
		impl_->eqgames[i].reset(nullptr);
	}
}

EQEmu::FileVerifyManager::~FileVerifyManager() {
	delete impl_;
}

bool EQEmu::FileVerifyManager::VerifySpellFile(const EQApplicationPacket *app, ClientVersion version) {
	if(!impl_->spell_data) {
		impl_->spell_data.reset(new FileVerify());

		if(!impl_->spell_data->Load("verify/spells_us.txt")) {
			return true;
		}
	}

	return impl_->spell_data->Verify((char*)app->pBuffer, app->size, version);
}

bool EQEmu::FileVerifyManager::VerifySkillFile(const EQApplicationPacket *app, ClientVersion version) {
	if(!impl_->skill_data) {
		impl_->skill_data.reset(new FileVerify());

		if(!impl_->skill_data->Load("verify/SkillCaps.txt")) {
			return true;
		}
	}

	return impl_->skill_data->Verify((char*)app->pBuffer, app->size, version);
}

bool EQEmu::FileVerifyManager::VerifyBaseDataFile(const EQApplicationPacket *app, ClientVersion version) {
	if(!impl_->base_data) {
		impl_->base_data.reset(new FileVerify());

		if(!impl_->base_data->Load("verify/BaseData.txt")) {
			return true;
		}
	}

	return impl_->base_data->Verify((char*)app->pBuffer, app->size, version);
}

bool EQEmu::FileVerifyManager::VerifyEQGame(const EQApplicationPacket *app, ClientVersion version) {
	int v = (int)version;
	if(v >= (int)ClientVersion::MaxClientVersions) {
		return true;
	}

	if(!impl_->eqgames[v]) {
		impl_->eqgames[v].reset(new FileVerify());

		if(!impl_->eqgames[v]->Load(StringFormat("verify/%s/eqgame.exe", ClientVersionName(version)).c_str())) {
			return true;
		}
	}

	return impl_->eqgames[v]->Verify((char*)app->pBuffer, app->size, version);
}
