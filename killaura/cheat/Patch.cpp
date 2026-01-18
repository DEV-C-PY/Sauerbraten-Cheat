#include "Patch.h"

namespace Patch {
	void Init() {
		ThroughWall::Init();
		Fly::Init();

		if (Cg_Func::H_Fly) Fly::CheckWall->Apply();
		if (Cg_Func::H_Collide) ThroughWall::Update();
	}
}