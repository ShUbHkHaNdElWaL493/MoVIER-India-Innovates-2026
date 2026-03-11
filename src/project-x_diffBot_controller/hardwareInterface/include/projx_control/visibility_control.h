
#ifndef PROKECTX_HW__VISIBILITY_CONTROL_H_
#define PROKECTX_HW__VISIBILITY_CONTROL_H_


#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define PROKECTX_HW_EXPORT __attribute__((dllexport))
#define PROKECTX_HW_IMPORT __attribute__((dllimport))
#else
#define PROKECTX_HW_EXPORT __declspec(dllexport)
#define PROKECTX_HW_IMPORT __declspec(dllimport)
#endif
#ifdef PROKECTX_HW_BUILDING_DLL
#define PROKECTX_HW_PUBLIC PROKECTX_HW_EXPORT
#else
#define PROKECTX_HW_PUBLIC PROKECTX_HW_IMPORT
#endif
#define PROKECTX_HW_PUBLIC_TYPE PROKECTX_HW_PUBLIC
#define PROKECTX_HW_LOCAL
#else
#define PROKECTX_HW_EXPORT __attribute__((visibility("default")))
#define PROKECTX_HW_IMPORT
#if __GNUC__ >= 4
#define PROKECTX_HW_PUBLIC __attribute__((visibility("default")))
#define PROKECTX_HW_LOCAL __attribute__((visibility("hidden")))
#else
#define PROKECTX_HW_PUBLIC
#define PROKECTX_HW_LOCAL
#endif
#define PROKECTX_HW_PUBLIC_TYPE
#endif

#endif  