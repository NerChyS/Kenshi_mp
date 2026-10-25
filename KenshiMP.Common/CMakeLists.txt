add_library(KenshiMP.Common STATIC
    src/protocol.cpp
    src/serialization.cpp
    src/compression.cpp
    src/config.cpp
)

target_include_directories(KenshiMP.Common PUBLIC include)
target_link_libraries(KenshiMP.Common PUBLIC nlohmann_json::nlohmann_json spdlog::spdlog)

# Async SEH so __try/__except catches faulting memory loads (MSVC /EHa semantics)
target_compile_options(KenshiMP.Common PRIVATE /EHa /Od)
