#pragma once

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include <wincon.h>
#include <stdio.h>
#include <iostream>
#include <string>

//
//	Manager for the Windows Console.
//	Redirects stdout and stdin to the console.
//	ReadConsole() and WriteConsole() continue to work.
//	

class ConsoleWindow {
public:
	HANDLE m_hConsoleIn = NULL;
	HANDLE m_hConsoleOut = NULL;
private:
	char sz[200] = "";
	char buffer[80] = "";
	const char* m_prompt = ">";

public:
	ConsoleWindow() {}
	~ConsoleWindow() {
		free();
	}

	bool isAllocated() const {
		return (m_hConsoleOut != NULL);
	}

	void allocate(LPCSTR lpConsoleTitle = NULL);
	bool userActivityPending();
	std::string getCommand();
	bool free();
};

