if(transpose_32)
  add_sources(dvbcsa_bs_transpose32.c)
endif()

if(transpose_64)
  add_sources(dvbcsa_bs_transpose64.c)
endif()

if(transpose_128)
  add_sources(dvbcsa_bs_transpose128.c)
endif()

add_sources(dvbcsa_algo.c dvbcsa_block.c dvbcsa_bs_algo.c dvbcsa_bs_block.c
  dvbcsa_bs_key.c dvbcsa_bs_stream.c dvbcsa_stream.c dvbcsa_bs.h dvbcsa_pv.h 
  dvbcsa_bs_uint64.h dvbcsa_bs_uint32.h dvbcsa_bs_mmx.h dvbcsa_bs_sse.h 
  dvbcsa_bs_altivec.h dvbcsa_bs_transpose.c dvbcsa_key.c)
