#pragma once

__declspec(dllexport) unsigned char __stdcall linear2alaw(int	pcm_val);
__declspec(dllexport) int __stdcall alaw2linear(unsigned char	a_val);
__declspec(dllexport) unsigned char __stdcall linear2ulaw(int		pcm_val);
__declspec(dllexport) int __stdcall ulaw2linear(unsigned char	u_val);
__declspec(dllexport) unsigned char __stdcall alaw2ulaw(unsigned char	aval);
__declspec(dllexport) unsigned char __stdcall ulaw2alaw(unsigned char	uval);
