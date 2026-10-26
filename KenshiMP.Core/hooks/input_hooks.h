#pragma once

namespace kmp::input_hooks {

bool Install();
void Uninstall();

// Printable character from WM_CHAR for our text fields (chat / menu edit box).
// De-duplicated against the same key seen through the OIS keyDownEvent hook.
void OnWndChar(wchar_t ch);

} // namespace kmp::input_hooks
