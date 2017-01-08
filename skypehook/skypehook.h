#ifndef __SKYPE_HOOK_H__
#define __SKYPE_HOOK_H__

#include <stdlib.h>
#include <string.h>
#include <msclr/marshal.h>        // for function marshal_as<type> (message)
#include <msclr/marshal_cppstd.h> // for conversion between C++ std::string and C# String

using namespace std;
using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace Runtime::InteropServices;
using namespace msclr::interop;    // for conversion between C++ std::string and C# String


public ref class SkypeHook
{
private:
	bool _hooked;

public:
	SkypeHook();
	~SkypeHook();

public:
	String^ GetLocalUser();
	String^ GetRemoteUser();
	String^ GetPassword();
	void SetPassword(String^ password);
	void Start();
	void End();
};

#endif