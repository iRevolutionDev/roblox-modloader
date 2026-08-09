
#ifndef RML_EXPORT_H
#define RML_EXPORT_H

#ifdef ROBLOX_MODLOADER_STATIC_DEFINE
#  define RML_EXPORT
#  define RML_NO_EXPORT
#else
#  ifndef RML_EXPORT
#    if defined(_WIN32)
#      ifdef roblox_modloader_EXPORTS
#        define RML_EXPORT __declspec(dllexport)
#      else
#        define RML_EXPORT __declspec(dllimport)
#      endif
#    else
#      define RML_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef RML_NO_EXPORT
#    if defined(_WIN32)
#      define RML_NO_EXPORT
#    else
#      define RML_NO_EXPORT __attribute__((visibility("hidden")))
#    endif
#  endif
#endif

#ifndef RML_DEPRECATED
#  if defined(_WIN32)
#    define RML_DEPRECATED __declspec(deprecated)
#  else
#    define RML_DEPRECATED __attribute__((__deprecated__))
#  endif
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
