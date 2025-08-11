#pragma once
#ifndef CROSS_IPC_EXPORT_H
#define CROSS_IPC_EXPORT_H

#ifdef _WIN32
#ifdef CROSS_IPC_EXPORTS
#define CROSS_IPC_API __declspec(dllexport)
#else
#define CROSS_IPC_API __declspec(dllimport)
#endif
#else
#define CROSS_IPC_API __attribute__((visibility("default")))
#endif

#endif // CROSS_IPC_EXPORT_H