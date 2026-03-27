
#ifndef RML_EXPORT_H
#define RML_EXPORT_H

#ifdef ROBLOX_MODLOADER_STATIC_DEFINE
#  define RML_EXPORT
#  define RML_NO_EXPORT
#else
#  ifndef RML_EXPORT
#    ifdef roblox_modloader_EXPORTS
        /* We are building this library */
#      define RML_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define RML_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef RML_NO_EXPORT
#    define RML_NO_EXPORT 
#  endif
#endif

#ifndef RML_DEPRECATED
#  define RML_DEPRECATED __declspec(deprecated)
#endif

#ifndef RML_DEPRECATED_EXPORT
#  define RML_DEPRECATED_EXPORT RML_EXPORT RML_DEPRECATED
#endif

#ifndef RML_DEPRECATED_NO_EXPORT
#  define RML_DEPRECATED_NO_EXPORT RML_NO_EXPORT RML_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef ROBLOX_MODLOADER_NO_DEPRECATED
#    define ROBLOX_MODLOADER_NO_DEPRECATED
#  endif
#endif

#endif /* RML_EXPORT_H */
