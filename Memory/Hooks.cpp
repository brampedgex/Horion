#include "Hooks.h"
#include "../Directx/Directx.h"

Hooks    g_Hooks;
bool firstTime = true;
void Hooks::Init()
{
	logF("Initting Hooks");
	// GameMode::tick Signature
	// CC 8B 41 ?? 89 41 ?? C3
	//    ^^ Function starts here
	void* func = reinterpret_cast<void*>(Utils::FindSignature("CC 8B 41 ?? 89 41 ?? C3") + 1);
	g_Hooks.gameMode_tickHook = std::make_unique<FuncHook>(func, Hooks::GameMode_tick);
	g_Hooks.gameMode_tickHook->init();

	// ChatScreenController::_sendChatMessage
	// 40 57 48 83 EC 20 48 83 B9 ?? ?? ?? ?? 00 48 8B F9 0F 85
	// ^^ 
	void* _sendChatMessage = reinterpret_cast<void*>(Utils::FindSignature("40 57 48 83 EC 20 48 83 B9 ?? ?? ?? ?? 00 48 8B F9 0F 85"));
	g_Hooks.chatScreen_sendMessageHook = std::make_unique<FuncHook>(_sendChatMessage, Hooks::ChatScreenController_sendChatMessage);
	g_Hooks.chatScreen_sendMessageHook->init();

	//IDXGISwapChain::present;
	// using vtable found with dummy thing
	void** swapChainVtable = static_cast<void**>(getSwapChain());
	void* presentFunc = swapChainVtable[8];
	g_Hooks.d3d11_presentHook = std::make_unique<FuncHook>(presentFunc, Hooks::d3d11_present);
	g_Hooks.d3d11_presentHook->init();

	// 
	void* _shit = reinterpret_cast<void*>(Utils::FindSignature("30 5F C3 CC 48 8B C4 55 56 57 41 54") + 4);
	g_Hooks.renderTextHook = std::make_unique<FuncHook>(_shit, Hooks::renderText);
	g_Hooks.renderTextHook->init();

	// I8n::get doesnt want to work
	//void *_shitshikt = reinterpret_cast<void*>(g_Data.getModule()->ptrBase + 0xB577C0);
	//g_Hooks.I8n_getHook = std::make_unique<FuncHook>(_shitshikt, Hooks::I8n_get);
	//g_Hooks.I8n_getHook->init();

	void *shat = reinterpret_cast<void*>(g_Data.getModule()->ptrBase + 0x6908A0);
	g_Hooks.Options_getVersionStringHook = std::make_unique<FuncHook>(shat, Hooks::Options_getVersionString);
	g_Hooks.Options_getVersionStringHook->init();
	logF("Hooks hooked");
}

void Hooks::Restore()
{
	g_Hooks.gameMode_tickHook->Restore();
	g_Hooks.chatScreen_sendMessageHook->Restore();
	g_Hooks.d3d11_presentHook->Restore();
	g_Hooks.renderTextHook->Restore();
	g_Hooks.Options_getVersionStringHook->Restore();
	//g_Hooks.I8n_getHook->Restore();
}

void __fastcall Hooks::GameMode_tick(C_GameMode * _this)
{
	static auto oTick = g_Hooks.gameMode_tickHook->GetOriginal<GameMode_tick_t>();
	oTick(_this); // Call Original Func
	GameData::updateGameData(_this);
}

void __fastcall Hooks::ChatScreenController_sendChatMessage(uint8_t * _this)
{
	static auto oSendMessage = g_Hooks.chatScreen_sendMessageHook->GetOriginal<ChatScreen_sendChatMessage_t>();
	uintptr_t* idk = reinterpret_cast<uintptr_t*>(_this + 0x688);
	if (*idk) {
		char* message = reinterpret_cast<char*>(_this + 0x678);
		if (*reinterpret_cast<__int64*>(_this + 0x690) >= 0x10)
			message = *reinterpret_cast<char**>(message);
		__int64* v6 = reinterpret_cast<__int64*>(_this + 0x690);

		if (*message == '.') {
			logF("Command: %s", message);
			logF("Yote %llX", message);
			//*message = 0x0; // Remove command in textbox
			//*v6 = 0x0;
			//*idk = 0x0;
			return;
		}
	}
	oSendMessage(_this);
}

struct Meinecraft {
	uintptr_t filler[20];
};

HRESULT __stdcall Hooks::d3d11_present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	static auto oPresent = g_Hooks.d3d11_presentHook->GetOriginal<d3d11_present_t>();
//	logF("swap %llX", pSwapChain);
	//Draw(pSwapChain);
	return oPresent(pSwapChain, SyncInterval, Flags);
}

struct BigCantWork {
	union {
		char boiii[16]; //0x0000 
		char *pText; //0x0000 
	};
	size_t length1;
	size_t length2;
};

__int64 __fastcall Hooks::renderText(__int64 yeet, __int64 yote) // I have no idea what this function is, only thing i know is that screencontext is in yote
{
	static auto oText = g_Hooks.renderTextHook->GetOriginal<renderText_t>();

	__int64 savedYote = yote;

	__int64 retval = oText(yeet, yote);

	yote = savedYote;

	using fillRectangle = void(__fastcall*)(uintptr_t, const float* rect, const float* color, float a4);
	static fillRectangle fill = reinterpret_cast<fillRectangle>(Utils::FindSignature("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 83 3D ?? ?? ?? ?? 00 49 8B F8 0F 29 74 24 ?? 48 8B DA 0F 28 F3 48 8B F1 75 2C B9 01 00 00 00 E8 ?? ?? ?? ?? 48 89 44 24 ?? 48 8B 0D ?? ?? ?? ?? 48 89 05 ?? ?? ?? ?? 48 85 C9 74 0A BA 01 00 00 00 E8 ?? ?? ?? ?? F3 0F 10 07 48"));

	float* reee = new float[4]; // Absolute Screen coordinates
	reee[0] = 0;    // startX
	reee[1] = 100;  //   endX
	reee[2] = 115;  // startY
	reee[3] = 135;  //   endY

	static float* col = new float[8];
	col[0] = 0.f;
	col[1] = 0.f;
	col[2] = 0.f;
	col[3] = 1;

	fill(yote, reee, col, 0.5f); // alpha

	col[0] = 0.3f;
	col[1] = 1;
	col[2] = 0.3f;
	col[3] = 1;

	static float* pos = new float[4];
	pos[0] = 0;
	pos[1] = 50;
	pos[2] = 120;
	pos[3] = 130;
	
	std::string textStr = std::string("My brain is big yes");

	static BigCantWork text;
	text.length1 = textStr.size();
	text.length2 = text.length1 | 0xF;

	if (textStr.size() >= 16) {
		char* ptr = static_cast<char*>(malloc(textStr.size() + 1));
		strcpy_s(ptr, textStr.size() + 1, textStr.c_str());
		text.pText = ptr;
	}
	else {
		strcpy_s(text.boiii, 16, textStr.c_str());
	}

	using drawText = void(__fastcall*)(uintptr_t, uintptr_t font, const float* pos, BigCantWork text, float* color, float alpha, DWORD textAlign, float* textMeasure, uintptr_t *caret);
	static drawText drawT = reinterpret_cast<drawText>(Utils::FindSignature("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8D 59 ?? 4C 8B D2 48 8B 8C 24"));
	static uintptr_t font = reinterpret_cast<uintptr_t>(g_Data.getClientInstance()->getFont());
	static float eins = 1.f;
	static uintptr_t oof = 0;
	memset(&oof, 0xFF, 4);
	
	drawT(yote, font, pos, text, col, 1, 0, &eins, &oof);

	using flushText = void(__fastcall*)(uintptr_t, float tim);
	flushText flushTextFunc = reinterpret_cast<flushText>(g_Data.getModule()->ptrBase + 0x858E70);
	flushTextFunc(yote, 0);

	return retval;
}

char* __fastcall Hooks::I8n_get(void* f, char* str)
{
	static auto oGet = g_Hooks.renderTextHook->GetOriginal<I8n_get_t>();

	//static char* shit = "yeet";

	//BigCantWork yote;
	//strcpy_s(yote.boiii, 16, shit);
	//yote.length1 = 4;
	//yote.length2 = 4;
	//yote.length3 = 4;

	//if (strcmp(str, "menu.play") == 0)
		//return &yote;
	
	return oGet(f, str);
}

TextHolder * __fastcall Hooks::Options_getVersionString(void * ya, __int64 idk)
{
	static auto oGetVer = g_Hooks.Options_getVersionStringHook->GetOriginal<Options_getVersionString_t>();
	TextHolder* version = oGetVer(ya, idk);
	version->inlineText[0] = 'E';
	return version;
}
