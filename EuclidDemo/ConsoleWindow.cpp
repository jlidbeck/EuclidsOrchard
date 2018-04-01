#pragma once
#include "ConsoleWindow.h"

void ConsoleWindow::allocate(LPCSTR lpConsoleTitle) {
	DWORD numEvents, numRead, numWritten;
	if(!m_hConsoleOut) {
		::AllocConsole();
		if(lpConsoleTitle != NULL) {
			::SetConsoleTitle(lpConsoleTitle);
		}
		m_hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

		// direct stdout, stdin to/from the console
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
		freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

		std::cout << m_prompt;
	}
}

bool ConsoleWindow::userActivityPending() {
	DWORD numEvents, numRead, numWritten;

	GetNumberOfConsoleInputEvents(m_hConsoleIn, &numEvents);
	if(!numEvents) return false;

	INPUT_RECORD inputRecords[10];
	while(PeekConsoleInput(m_hConsoleIn, inputRecords, 1, &numRead)) {
		if(!numEvents || !numRead) {
			return false;
		}

		if(inputRecords[0].EventType == KEY_EVENT) {
			return true;
		}	// if key event
		else {
			// consume and ignore non-key events
			ReadConsoleInput(m_hConsoleIn, inputRecords, 1, &numRead);
		}
	}

	return false;
}

std::string ConsoleWindow::getCommand() {
	DWORD numEvents, numRead, numWritten;
	std::string sz;

	allocate();

	GetNumberOfConsoleInputEvents(m_hConsoleIn, &numEvents);
	if(!numEvents) return sz;

	INPUT_RECORD inputRecords[10];
	//PeekConsoleInput(m_hConsoleIn, inputRecords, 10, &numRead);
	while(PeekConsoleInput(m_hConsoleIn, inputRecords, 1, &numRead) ) {
		if(!numEvents || !numRead) break;

		if(inputRecords[0].EventType == KEY_EVENT) {
			// handle key events.
			// since we only have the first key, we'll wait and read an entire line in.
			// The ReadConsole call is synchronous, and the program will halt until the user hits EOL

			// TODO: if modifier (alt, ctrl..) break out of this.. we want to allow the user to get away without freezing everything

			//printf("Records: %d %d %c\n", numRead, inputRecords[0].EventType, inputRecords[0].Event.KeyEvent.uChar.UnicodeChar);
			printf(">");

			// for writing, instead of this:
			//WriteConsole(m_hConsoleOut, sz, strlen(sz), &numWritten, NULL);
			// you can do this:
			//std::cout << "Console active\n" << m_prompt;
			//printf("Hello %s", world);

			// instead of this:
			//ReadConsole(m_hConsoleIn, buffer, sizeof(buffer) - 1, &numRead, NULL);
			//buffer[numRead] = '\0';

			// you can use this:
			gets_s(buffer);
			// or this:
			//std::cin >> buffer;	// up to whitespace

			int i = strlen(buffer);
			for(; i > 0 && isspace(buffer[i - 1]); --i);
			buffer[i] = '\0';
			if(i > 0) {
				//WriteConsole(m_hConsoleOut, sz, strlen(sz), &numWritten, NULL);
				//printf("%d events, read: %s\n%s", numEvents, buffer, m_prompt);
				return buffer;
			}
			else {
				// empty line, just EOL
				//std::cout << m_prompt;
			}
		}	// if key event
		else {
			// consume and ignore non-key events
			ReadConsoleInput(m_hConsoleIn, inputRecords, 1, &numRead);
			//printf("{%d}", inputRecords[0].EventType);
		}
	}

	return sz;
}

bool ConsoleWindow::free() {
	return FreeConsole();
}
