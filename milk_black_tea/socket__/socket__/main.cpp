// main.cpp: �D�n�M���ɡC

#include "stdafx.h"
#include "Form1.h"
#include "Start.h"

using namespace socket__;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// �إߥ��󱱨���e�A���ҥ� Windows XP ��ı�ƮĪG
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// �إߥD�����ð���
	Application::Run(gcnew Start());
	return 0;
}
