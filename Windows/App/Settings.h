// KuocKey's windows (1:1 with the macOS app): main window, onboarding, About, Acknowledgements, typing log.
#pragma once
#include <windows.h>
namespace kuockey { namespace ui {
void showSettings(HINSTANCE inst, bool firstRun);   // firstRun shows the onboarding instead
void showOnboarding(HINSTANCE inst);
void showAbout(HINSTANCE inst);
void showAcknowledgements(HINSTANCE inst);
void showTypingLog(HINSTANCE inst);
void showMacros(HINSTANCE inst);   // Bảng gõ tắt: the shortcut table, with CSV import and export
void languageChanged(int lang);
}}
