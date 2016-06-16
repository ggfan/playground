##################################################################################
# neon is not used


cmake_minimum_required(VERSION 3.4.1)
set (CMAKE_VERBOSE_MAKEFILE on)

set(WEBP_CFLAGS "-Wall -DANDROID -DHAVE_MALLOC_H -DHAVE_PTHREAD -DWEBP_USE_THREAD")
# ifeq ($(APP_OPTIM),release)
#  WEBP_CFLAGS += -finline-functions -ffast-math \
#                 -ffunction-sections -fdata-sections
#  ifeq ($(findstring clang,$(NDK_TOOLCHAIN_VERSION)),)
#    WEBP_CFLAGS += -frename-registers -s
#  endif
# endif
#ifneq ($(findstring armeabi-v7a, $(TARGET_ARCH_ABI)),)
  # Setting LOCAL_ARM_NEON will enable -mfpu=neon which may cause illegal
  # instructions to be generated for armv7a code. Instead target the neon code
  # specifically.
#  NEON := c.neon
#  USE_CPUFEATURES := yes
#else
#  NEON := c
#endif

# not using neon for now
set(NEON c)

#######Source Code ###########################
set(dsp_dec_srcs 
    ../src/src/dsp/alpha_processing.c
    ../src/src/dsp/alpha_processing_mips_dsp_r2.c
    ../src/src/dsp/alpha_processing_sse2.c
    ../src/src/dsp/alpha_processing_sse41.c
    ../src/src/dsp/argb.c
    ../src/src/dsp/argb_mips_dsp_r2.c
    ../src/src/dsp/argb_sse2.c
    ../src/src/dsp/cpu.c
    ../src/src/dsp/dec.c
    ../src/src/dsp/dec_clip_tables.c
    ../src/src/dsp/dec_mips32.c
    ../src/src/dsp/dec_mips_dsp_r2.c
    ../src/src/dsp/dec_msa.c
    ../src/src/dsp/dec_neon.${NEON}
    ../src/src/dsp/dec_sse2.c
    ../src/src/dsp/dec_sse41.c
    ../src/src/dsp/filters.c
    ../src/src/dsp/filters_mips_dsp_r2.c
    ../src/src/dsp/filters_sse2.c
    ../src/src/dsp/lossless.c
    ../src/src/dsp/lossless_mips_dsp_r2.c
    ../src/src/dsp/lossless_neon.${NEON}
    ../src/src/dsp/lossless_sse2.c
    ../src/src/dsp/rescaler.c
    ../src/src/dsp/rescaler_mips32.c
    ../src/src/dsp/rescaler_mips_dsp_r2.c
    ../src/src/dsp/rescaler_neon.${NEON}
    ../src/src/dsp/rescaler_sse2.c
    ../src/src/dsp/upsampling.c
    ../src/src/dsp/upsampling_mips_dsp_r2.c
    ../src/src/dsp/upsampling_neon.${NEON}
    ../src/src/dsp/upsampling_sse2.c
    ../src/src/dsp/yuv.c
    ../src/src/dsp/yuv_mips32.c
    ../src/src/dsp/yuv_mips_dsp_r2.c
    ../src/src/dsp/yuv_sse2.c)

set(dsp_enc_srcs 
    ../src/src/dsp/cost.c
    ../src/src/dsp/cost_mips32.c
    ../src/src/dsp/cost_mips_dsp_r2.c
    ../src/src/dsp/cost_sse2.c
    ../src/src/dsp/enc.c
    ../src/src/dsp/enc_avx2.c
    ../src/src/dsp/enc_mips32.c
    ../src/src/dsp/enc_mips_dsp_r2.c
    ../src/src/dsp/enc_neon.${NEON}
    ../src/src/dsp/enc_sse2.c
    ../src/src/dsp/enc_sse41.c
    ../src/src/dsp/lossless_enc.c
    ../src/src/dsp/lossless_enc_mips32.c
    ../src/src/dsp/lossless_enc_mips_dsp_r2.c
    ../src/src/dsp/lossless_enc_neon.${NEON}
    ../src/src/dsp/lossless_enc_sse2.c
    ../src/src/dsp/lossless_enc_sse41.c)

set(dec_srcs
    ../src/src/dec/alpha.c
    ../src/src/dec/buffer.c
    ../src/src/dec/frame.c
    ../src/src/dec/idec.c
    ../src/src/dec/io.c
    ../src/src/dec/quant.c
    ../src/src/dec/tree.c
    ../src/src/dec/vp8.c
    ../src/src/dec/vp8l.c
    ../src/src/dec/webp.c)
set(enc_srcs
    ../src/src/enc/alpha.c
    ../src/src/enc/analysis.c
    ../src/src/enc/backward_references.c
    ../src/src/enc/config.c
    ../src/src/enc/cost.c
    ../src/src/enc/delta_palettization.c
    ../src/src/enc/filter.c
    ../src/src/enc/frame.c
    ../src/src/enc/histogram.c
    ../src/src/enc/iterator.c
    ../src/src/enc/near_lossless.c
    ../src/src/enc/picture.c
    ../src/src/enc/picture_csp.c
    ../src/src/enc/picture_psnr.c
    ../src/src/enc/picture_rescale.c
    ../src/src/enc/picture_tools.c
    ../src/src/enc/quant.c
    ../src/src/enc/syntax.c
    ../src/src/enc/token.c
    ../src/src/enc/tree.c
    ../src/src/enc/vp8l.c
    ../src/src/enc/webpenc.c)

set(utils_dec_srcs
    ../src/src/utils/bit_reader.c
    ../src/src/utils/color_cache.c
    ../src/src/utils/filters.c
    ../src/src/utils/huffman.c
    ../src/src/utils/quant_levels_dec.c
    ../src/src/utils/random.c
    ../src/src/utils/rescaler.c
    ../src/src/utils/thread.c
    ../src/src/utils/utils.c)
set(utils_enc_srcs
    ../src/src/utils/bit_writer.c
    ../src/src/utils/huffman_encode.c
    ../src/src/utils/quant_levels.c)

# need cpufeatures when we begin to use neon
include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
add_library(cpufeatures STATIC
            ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)

#######build webpdecoder_static, which is needed for webp
set(webpdecoder_static_SRCS
    ${dec_srcs}
    ${dsp_dec_srcs}
    ${utils_dec_srcs})

# WEBP_SWAP_16BIT_CSP: needed to swap the endianess
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WEBP_CFLAGS} -DWEBP_SWAP_16BIT_CSP")
include_directories(CMAKE_CURRENT_SOURCE_DIR/../src/src)

# prefer arm over thumb mode for performance gains
#LOCAL_ARM_MODE := arm

#ifeq ($(USE_CPUFEATURES),yes)
#  LOCAL_STATIC_LIBRARIES := cpufeatures
#endif


add_library(webpdecoder_static STATIC ${webpdecoder_static_SRCS})

################################################################
# build for webp lib
set(webp_SRCS ${dsp_enc_srcs} ${enc_srcs} ${utils_enc_srcs})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WEBP_CFLAGS}")
include_directories({CMAKE_CURRENT_SOURCE_DIR}/../src/src)

# how to handle this???: prefer arm over thumb mode for performance gains
# LOCAL_ARM_MODE := arm

#add_library(webp SAHRED ${webp_SRCS})
add_library(webp STATIC ${webp_SRCS})
target_link_libraries(webp webpdecoder_static)

