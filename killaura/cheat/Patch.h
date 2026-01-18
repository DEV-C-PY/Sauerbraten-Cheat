#pragma once
#include "../Manager/HookManager.h"
#include "func_prototype.h"
#include "../Manager/ConfigManager.h"
#include "cheat.h"

//存放函数
namespace Patch {
	void Init();
}
//存放补丁实际内容
namespace Patch {
	//穿墙功能相关的补丁
	namespace ThroughWall {
		//应用于Move函数内部
		//作用是屏蔽水平向墙体的检测和
		//强制应用向量（假如向量.z = -100，函数会对这个向量进行合法性判断，如果踩着地板我们的位置就不会被改变）
		inline MemoryPatch* patchXY = nullptr;

		//应用于ModifyGravity函数内部
		//作用是屏蔽函数内的纵轴检测，让函数无脑累加下坠加速值（此函数产出的向量会影响Move函数）
		inline MemoryPatch* patchZ = nullptr;

		//应用于switchfloor函数内部，因为只需要写入补丁不需要hook，所以没分析这个函数的原型
		//作用是防止下坠速度复位，也就是z向量复位
		inline MemoryPatch* path_RestZ1 = nullptr;
		inline MemoryPatch* path_RestZ2 = nullptr;

		//补丁初始化函数
		inline void Init() {
			//补丁内容就是nop掉判断
			//偏移和补丁内容直接硬编码了，因为不需要修改，而且也仅在这里使用一次
			patchXY = new MemoryPatch((uintptr_t)func::Move + 0x89, { 0x90, 0x90, 0x90, 0x90, 0x90,0x90 });
			patchZ = new MemoryPatch((uintptr_t)func::ModifyGravity + 0x4D, { 0x90, 0x90 });
			path_RestZ1 = new MemoryPatch((uintptr_t)ct_baseAddr::gameBase + 0x110E00 + 0x1000, { 0x90,0x90,0x90 });
			path_RestZ2 = new MemoryPatch((uintptr_t)ct_baseAddr::gameBase + 0x111036 + 0x1000, { 0x90, 0x90, 0x90, 0x90, 0x90,0x90 });
		}

		//取消所有补丁
		inline void AllDone() {
			if (patchXY->IsApplied()) patchXY->Restore();
			if (patchZ && patchZ->IsApplied()) patchZ->Restore();
			if (path_RestZ1 && path_RestZ1->IsApplied()) path_RestZ1->Restore();
			if (path_RestZ2 && path_RestZ2->IsApplied()) path_RestZ2->Restore();
		}

		//更新配置函数
		inline void Update() {
			//如果两个方向的穿墙都开启着，那我们直接去下面的逻辑
			//因为XY补丁，两个方向都需要用
			if (Cg_Move::IsXY_through()) {
				if (patchXY && !patchXY->IsApplied()) patchXY->Apply();
				if (patchZ && patchZ->IsApplied()) patchZ->Restore();
				if (path_RestZ1 && path_RestZ1->IsApplied()) path_RestZ1->Restore();
				if (path_RestZ2 && path_RestZ2->IsApplied()) path_RestZ2->Restore();
			} 
			else if (Cg_Move::IsZ_through()) {
				if (patchXY && !patchXY->IsApplied()) patchXY->Apply();
				if (patchZ && !patchZ->IsApplied()) patchZ->Apply();
				if (path_RestZ1 && !path_RestZ1->IsApplied()) path_RestZ1->Apply();
				if (path_RestZ2 && !path_RestZ2->IsApplied()) path_RestZ2->Apply();
			}
			else if (Cg_Move::IsALL_through()) AllDone();//全向穿墙由Collide负责完成
		}

	}
	//飞行功能补丁
	namespace Fly {
		//应用于MovePlayer函数内
		//此补丁的作用是，强制开启墙体检查。否则开启飞行模式我们就会穿墙了，我们希望穿墙由我们自己控制
		inline MemoryPatch* CheckWall = nullptr;

		//飞行功能补丁初始化
		inline void Init() {
			//改为Jmp
			CheckWall = new MemoryPatch(((uintptr_t)func::MovePlayer + 0x183), { 0xEB });
		}
	}
}
