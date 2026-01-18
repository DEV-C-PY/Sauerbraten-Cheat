#include "HookManager.h"
#include <exception>

BYTE HookManager::int3 = 0xCC;//int3机器码0xCC
BYTE HookManager::WhiteMem = 0x00;//add byte ptr ds:[rax], al
bool HookManager::WriteMemory(void* dest, const void* src, size_t len) {
    DWORD oldProtect;
    if (!VirtualProtect(dest, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    memcpy(dest, src, len);
    FlushInstructionCache(GetCurrentProcess(), dest, len);
    VirtualProtect(dest, len, oldProtect, &oldProtect);
    return true;
}
bool HookManager::InstallCallHook(uintptr_t hookAddr, uintptr_t targetFunc,
    uint8_t* originalBytes, size_t* patchSize) {
    // 计算偏移
    int64_t offset = static_cast<int64_t>(targetFunc) -
        static_cast<int64_t>(hookAddr + 5);

#ifdef _WIN64
    // 64位：检查是否需要绝对跳转
    if (offset < INT32_MIN || offset > INT32_MAX) {
        // 12字节绝对call: mov rax, addr; call rax
        if (originalBytes)
            memcpy(originalBytes, (void*)hookAddr, 12);
        if (patchSize)
            *patchSize = 12;

        uint8_t patch[12];
        patch[0] = 0x48; patch[1] = 0xB8;
        *(uint64_t*)&patch[2] = targetFunc;
        patch[10] = 0xFF; patch[11] = 0xD0;

        return WriteMemory((void*)hookAddr, patch, 12);
    }
#endif
    // 5字节相对call
    if (originalBytes)
        memcpy(originalBytes, (void*)hookAddr, 5);
    if (patchSize)
        *patchSize = 5;

    uint8_t patch[5] = { 0xE8 };
    *(int32_t*)&patch[1] = static_cast<int32_t>(offset);

    return WriteMemory((void*)hookAddr, patch, 5);
}
bool HookManager::RestoreHook(uintptr_t hookAddr, const uint8_t* originalBytes,
    size_t len) {
    return WriteMemory((void*)hookAddr, originalBytes, len);
}
void* HookManager::FindBlankMem(uintptr_t strAddr, size_t blank_len, size_t& act_len) {
    void* RetAddr = 0x0;//返回找到的空白内存
    try {
        BYTE* machine_code = (BYTE*)(strAddr);
        act_len = 0;
        BYTE code{};
        for (DWORD i = 0; i < INT_MAX; ++i) {
            if (machine_code[i] == int3 || machine_code[i] == WhiteMem) {
                code = machine_code[i];
                for (int j = 0; j < blank_len; ++j) {
                    if (machine_code[i + j] == code) ++act_len;
                    else {
                        i = i + j;//让外层循环更新，防止重复遍历，浪费性能
                        break;
                    }
                }
                if (act_len >= blank_len) {
                    RetAddr = reinterpret_cast<void*>(strAddr + i);
                    break;
                }
                act_len = 0;
            }
        }
    }
    catch (std::exception e) {
        MessageBoxA(0, "查找内存空白错误！", e.what(), 0);
        RetAddr = nullptr;
    }

    return RetAddr;
}
bool HookManager::Bytes5ToJmp(uintptr_t o_tar, uintptr_t p_target) {
    try {
        BYTE code[5] = { 0xE9,0,0,0,0 };//0xE9:相对跳转指令
        int offset = static_cast<int>(o_tar - p_target);
        if (offset == 0) return false;//无意义的操作，跳转到自己

        offset *= -1;//相对于o_的位置！

        offset -= 5;//对于跳转，我们需要以地址+5字节指令的位置计算

        code[1] = (offset >> 0) & 0xFF;//小端存储，低位在前
        code[2] = (offset >> 8) & 0xFF;
        code[3] = (offset >> 16) & 0xFF;
        code[4] = (offset >> 24) & 0xFF;

        //替换原本的指令为jmp p_target
        WriteMemoryPatch(code, (BYTE*)(o_tar), 5);
    }
    catch (std::exception e) {
        MessageBoxA(0, "Bytes5ToJmp执行失败", e.what(), 0);
        return false;
    }

    return true;
}
bool HookManager::Bytes5ToCall(uintptr_t o_tar, uintptr_t p_target) {
    try {
        BYTE code[5] = { 0xE8,0,0,0,0 };//0xE8:相对call指令
        int offset = static_cast<int>(o_tar - p_target);
        if (offset == 0)return false;//无意义的操作，call自己

        offset *= -1;//相对于o_的位置！

        offset -= 5;//对于call，我们需要以地址+5字节指令的位置计算

        code[1] = (offset >> 0) & 0xFF;//小端存储，低位在前
        code[2] = (offset >> 8) & 0xFF;
        code[3] = (offset >> 16) & 0xFF;
        code[4] = (offset >> 24) & 0xFF;

        //替换原本的指令为call p_target
        WriteMemoryPatch(code, (BYTE*)(o_tar), 5);
    }
    catch (std::exception e) {
        MessageBoxA(0, "Bytes5ToCall执行失败", e.what(), 0);
        return false;
    }

    return true;
}
void HookManager::WriteMemoryPatch(BYTE* replace, BYTE* addr, size_t len) {
    //修改内存保护
    DWORD oldProtect;
    if (VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) {

        //写入补丁
        memcpy(addr, replace, len);

        //恢复内存保护
        VirtualProtect(addr, len, oldProtect, &oldProtect);
    }
}

//内存补丁管理类
//构造函数  会自动记录原本机器码
//1.写入地址    2.补丁内容
MemoryPatch::MemoryPatch(uintptr_t targetAddr, std::vector<BYTE> patchBytes)
    : _targetAddr((void*)targetAddr), _patchBytes(patchBytes)
{
    // 构造时，自动读取并保存原始字节
    _originalBytes.resize(patchBytes.size());
    memcpy(_originalBytes.data(), _targetAddr, patchBytes.size());
}
//应用补丁
void MemoryPatch::Apply() {
    if (_isApplied) return;

    DWORD oldProtect;
    VirtualProtect(_targetAddr, _patchBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(_targetAddr, _patchBytes.data(), _patchBytes.size());
    VirtualProtect(_targetAddr, _patchBytes.size(), oldProtect, &oldProtect);

    _isApplied = true;
}
//取消应用补丁
void MemoryPatch::Restore() {
    if (!_isApplied) return;

    DWORD oldProtect;
    VirtualProtect(_targetAddr, _originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(_targetAddr, _originalBytes.data(), _originalBytes.size()); // 恢复原始字节
    VirtualProtect(_targetAddr, _originalBytes.size(), oldProtect, &oldProtect);

    _isApplied = false;
}