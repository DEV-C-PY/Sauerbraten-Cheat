#include "func_prototype.h"
#include "cheat.h"

namespace func {
	void Init() {
		// 初始化函数指针
		func::IntersectRay = (tIntersectRay)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_IntersectRay);
		func::BulletTrace = (tBulletTrace)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_bulletTrace);
		func::Shoot = (tShoot)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_shoot);
		func::FireEvent = (tFireEvent)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_FireEvent);
		func::MovePlayer = (tMovePlayer)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_MovePlayer);
		func::Collide = (tCollide)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_Collide);
		func::ModifyGravity = (tModifyGravity)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_ModifyGravity);
		func::Move = (tMove)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_Move);
		func::ModifyVec = (tModifyVec)((uintptr_t)ct_baseAddr::gameBase + fc_offsets::func_ModifyVec);
	}
}
