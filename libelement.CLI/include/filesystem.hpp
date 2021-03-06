#pragma once

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && !defined(ELEMENT_FORCE_FALLBACK_FS)
    #if __has_include(<filesystem>)
        #define GHC_USE_STD_FS
        #include <filesystem>
namespace fs = std::filesystem;
    #endif
#endif

#ifndef GHC_USE_STD_FS
    #include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#endif