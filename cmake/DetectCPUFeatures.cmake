
function(detect_cpu_features)
try_run(RUN_RESULT COMPILE_RESULT
    ${CMAKE_CURRENT_BINARY_DIR}/features
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu_features.cpp
    COMPILE_DEFINITIONS -DCPU_FEATURES_BUILD_MAIN=1 -I${CMAKE_CURRENT_SOURCE_DIR}/include
    RUN_OUTPUT_VARIABLE RUN_OUTPUT
    COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT)
if (${COMPILE_RESULT} STREQUAL "TRUE")
    if (${RUN_RESULT} EQUAL 0)
        foreach(feature ${RUN_OUTPUT})
            # sse2 avx2 avx512f
            if (${feature} STREQUAL "sse2")
                set(FINGERA_HAS_SSE2 true PARENT_SCOPE)
            elseif (${feature} STREQUAL "avx2")
                set(FINGERA_HAS_AVX2 true PARENT_SCOPE)
            elseif (${feature} STREQUAL "avx512f")
                set(FINGERA_HAS_AVX512F true PARENT_SCOPE)
            endif()
        endforeach()
    else()
        message("Run cpu_feature failure result:" ${RUN_RESULT})
        message(${RUN_OUTPUT})
    endif()
else()
    message("Compile cpu_feature failure")
    message(${COMPILE_OUTPUT})
endif()
endfunction(detect_cpu_features)