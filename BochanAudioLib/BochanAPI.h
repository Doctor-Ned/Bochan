#pragma once

#ifdef BOCHAN_EXPORTS
#define BOCHANAPI __declspec(dllexport)
#elif defined(BOCHAN_EXPORTS_STATIC)
#define BOCHANAPI
#else
#define BOCHANAPI __declspec(dllimport)
#endif