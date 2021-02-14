#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <vector>
#include <string>

const std::wstring separator = L"@@@";

struct S_WINDOW {
	std::wstring title;
	HWND hwnd;
	RECT rect;
};

BOOL CALLBACK recognizeWindow(HWND hwnd,LPARAM lParam);
std::vector<HWND> recognizeWindows();
std::wstring getWindowTitle(HWND hwnd);
HWND getHwndByTitle(std::wstring title);
void printDumpedWindows(std::vector<S_WINDOW>* windows);
std::vector<S_WINDOW> restoreDumpedWindows();
std::vector<S_WINDOW> dumpWindows(std::vector<HWND>* windows);
void restoreWindows(std::vector<S_WINDOW>* windows);

int main(int argc, const char* argv[]) {
	if (argc > 1 && strcmp(argv[1], "-h") == 0 || argc == 1) {
		std::wcout << L"Usage: " << argv[0] << L" <action>" << std::endl
			<< L"-h for help" << std::endl
			<< L"-d to dump windows to stdout" << std::endl
			<< L"-r start reading windows to restore from stdin" << std::endl;
		return 0;
	}
	if (strcmp(argv[1], "-d") == 0) {
		printDumpedWindows(
			&dumpWindows(
				&recognizeWindows()
			)
		);
	}
	if (strcmp(argv[1], "-r") == 0) {
		std::vector<S_WINDOW> windows = restoreDumpedWindows();
		restoreWindows(&windows);
	}
}

std::vector<HWND> recognizeWindows() {
	std::vector<HWND> windows;
	EnumWindows(recognizeWindow, reinterpret_cast<LPARAM>(&windows));
	return windows;
}

BOOL CALLBACK recognizeWindow(HWND hwnd, LPARAM lParam) {
	if (!IsWindowVisible(hwnd)) {
		return TRUE;
	}
	std::vector<HWND>& windows = *reinterpret_cast<std::vector<HWND>*>(lParam);
	windows.push_back(hwnd);
	return TRUE;
}

std::wstring getWindowTitle(HWND hwnd) {
	const DWORD TITLE_SIZE = 1024;
	WCHAR windowTitle[TITLE_SIZE];
	GetWindowTextW(hwnd, windowTitle, TITLE_SIZE);
	return std::wstring(windowTitle);
}

std::vector<S_WINDOW> dumpWindows(std::vector<HWND>* windows) {
	std::vector<S_WINDOW> dumpedWindows;
	for (const auto& window : *windows) {
		RECT rect;
		GetWindowRect(window, &rect);

		S_WINDOW dumpWindow;
		dumpWindow.hwnd = window;
		dumpWindow.title = getWindowTitle(window);
		dumpWindow.rect = rect;

		dumpedWindows.push_back(dumpWindow);
	}

	return dumpedWindows;
}

void restoreWindows(std::vector<S_WINDOW>* windows) {
	for (S_WINDOW window : *windows) {
		if (window.hwnd == NULL) {
			window.hwnd = getHwndByTitle(window.title);
		}
		if (window.hwnd == NULL) {
			continue;
		}
		SetWindowPos(window.hwnd, 
			HWND_TOP, 
			window.rect.left, 
			window.rect.top, 
			window.rect.right, 
			window.rect.bottom,
			SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

HWND getHwndByTitle(std::wstring title) {
	std::vector<HWND> windows = recognizeWindows();
	for (const auto& window : windows) {
		if (title == getWindowTitle(window)) {
			return window;
		}
	}
	return NULL;
}

void printDumpedWindows(std::vector<S_WINDOW>* windows) {
	for (const auto& window : *windows) {
		if (window.title.empty()) {
			continue;
		}
		std::wcout <<
			window.title <<
			separator <<
			window.rect.bottom <<
			separator <<
			window.rect.left <<
			separator <<
			window.rect.right <<
			separator <<
			window.rect.top <<
			std::endl;
	}
}

std::vector<std::wstring> wsplit(std::wstring s, std::wstring sep) {
	std::vector<std::wstring> tokens;
	size_t pos = 0;
	std::wstring token;
	while ((pos = s.find(sep)) != std::string::npos) {
		token = s.substr(0, pos);
		tokens.push_back(token);
		s.erase(0, pos + sep.length());
	}
	tokens.push_back(s);
	return tokens;
}

std::vector<S_WINDOW> restoreDumpedWindows() {
	std::vector<S_WINDOW> windows;
	std::wstring buffer;
	while (std::getline(std::wcin, buffer)) {
		S_WINDOW window;
		std::vector<std::wstring> data = wsplit(buffer, separator);
		if (data.size() != 5) {
			break;
		}
		window.title = data[0];
		window.rect.bottom = std::stoi(data[1]);
		window.rect.left = std::stoi(data[2]);
		window.rect.right = std::stoi(data[3]);
		window.rect.top = std::stoi(data[4]);
		window.hwnd = NULL;
		windows.push_back(window);
	}
	return windows;
}