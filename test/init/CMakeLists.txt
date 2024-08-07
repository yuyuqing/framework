PROJECT (init)

MESSAGE(STATUS "in init project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "in init project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(dpdk_incs
    /home/wrk1/dpdk_install/include)

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../../pub)

set(base_incs
    ${PROJECT_SOURCE_DIR}/../../base/include)

set(dpdk_adapt_incs
    ${PROJECT_SOURCE_DIR}/../../dpdk_adapt/include)

include_directories(${dpdk_incs} 
                    ${pub_incs} 
                    ${base_incs} 
                    ${dpdk_adapt_incs})

set(lib_obj
    sub::libbase
    sub::libdpdk_adapt)

set(ENV{PKG_CONFIG_PATH} "/home/wrk1/dpdk_install/lib/x86_64-linux-gnu/pkgconfig")
find_package(PkgConfig REQUIRED)
message("PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")
pkg_check_modules(LIBDPDK REQUIRED libdpdk)

message("LIBDPDK_INCLUDE_DIRS     : ${LIBDPDK_INCLUDE_DIRS}")
message("LIBDPDK_LIBRARIES_DIRS   : ${LIBDPDK_LIBRARIES_DIRS}")
message("LIBDPDK_LIBRARIES        : ${LIBDPDK_LIBRARIES}")
message("LIBDPDK_STATIC_LIBRARIES : ${LIBDPDK_STATIC_LIBRARIES}")
message("LIBDPDK_STATIC_LDFLAGS   : ${LIBDPDK_STATIC_LDFLAGS}")

set(main_init_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/main_init.cpp)

add_executable(main_init ${main_init_srcs})
redefine_file_macro(main_init)
target_link_libraries(main_init -Wl,--whole-archive; ${lib_obj} -L/home/wrk1/dpdk_install/lib/x86_64-linux-gnu;
                      -l:librte_common_cpt.a;-l:librte_common_dpaax.a;-l:librte_common_iavf.a;-l:librte_common_idpf.a;
                      -l:librte_common_octeontx.a;-l:librte_bus_auxiliary.a;-l:librte_bus_cdx.a;-l:librte_bus_dpaa.a;
                      -l:librte_bus_fslmc.a;-l:librte_bus_ifpga.a;-l:librte_bus_pci.a;-l:librte_bus_platform.a;
                      -l:librte_bus_vdev.a;-l:librte_bus_vmbus.a;-l:librte_common_cnxk.a;-l:librte_common_qat.a;
                      -l:librte_common_sfc_efx.a;-l:librte_mempool_bucket.a;-l:librte_mempool_cnxk.a;-l:librte_mempool_dpaa.a;
                      -l:librte_mempool_dpaa2.a;-l:librte_mempool_octeontx.a;-l:librte_mempool_ring.a;-l:librte_mempool_stack.a;
                      -l:librte_dma_cnxk.a;-l:librte_dma_dpaa.a;-l:librte_dma_dpaa2.a;-l:librte_dma_hisilicon.a;
                      -l:librte_dma_idxd.a;-l:librte_dma_ioat.a;-l:librte_dma_skeleton.a;-l:librte_net_af_packet.a;
                      -l:librte_net_ark.a;-l:librte_net_atlantic.a;-l:librte_net_avp.a;-l:librte_net_axgbe.a;-l:librte_net_bnxt.a;
                      -l:librte_net_bond.a;-l:librte_net_cnxk.a;-l:librte_net_cpfl.a;-l:librte_net_cxgbe.a;-l:librte_net_dpaa.a;
                      -l:librte_net_dpaa2.a;-l:librte_net_e1000.a;-l:librte_net_ena.a;-l:librte_net_enetc.a;-l:librte_net_enetfec.a;
                      -l:librte_net_enic.a;-l:librte_net_failsafe.a;-l:librte_net_fm10k.a;-l:librte_net_gve.a;-l:librte_net_hinic.a;
                      -l:librte_net_hns3.a;-l:librte_net_i40e.a;-l:librte_net_iavf.a;-l:librte_net_ice.a;-l:librte_net_idpf.a;
                      -l:librte_net_igc.a;-l:librte_net_ionic.a;-l:librte_net_ixgbe.a;-l:librte_net_memif.a;-l:librte_net_netvsc.a;
                      -l:librte_net_nfp.a;-l:librte_net_ngbe.a;-l:librte_net_null.a;-l:librte_net_octeontx.a;-l:librte_net_octeon_ep.a;
                      -l:librte_net_pcap.a;-l:librte_net_pfe.a;-l:librte_net_qede.a;-l:librte_net_ring.a;-l:librte_net_softnic.a;
                      -l:librte_net_tap.a;-l:librte_net_thunderx.a;-l:librte_net_txgbe.a;-l:librte_net_vdev_netvsc.a;-l:librte_net_vhost.a;
                      -l:librte_net_virtio.a;-l:librte_net_vmxnet3.a;-l:librte_raw_cnxk_bphy.a;-l:librte_raw_cnxk_gpio.a;
                      -l:librte_raw_dpaa2_cmdif.a;-l:librte_raw_ntb.a;-l:librte_raw_skeleton.a;-l:librte_crypto_bcmfs.a;
                      -l:librte_crypto_caam_jr.a;-l:librte_crypto_cnxk.a;-l:librte_crypto_dpaa_sec.a;-l:librte_crypto_dpaa2_sec.a;
                      -l:librte_crypto_nitrox.a;-l:librte_crypto_null.a;-l:librte_crypto_octeontx.a;-l:librte_crypto_scheduler.a;
                      -l:librte_crypto_virtio.a;-l:librte_compress_octeontx.a;-l:librte_regex_cn9k.a;-l:librte_ml_cnxk.a;
                      -l:librte_vdpa_ifc.a;-l:librte_vdpa_sfc.a;-l:librte_event_cnxk.a;-l:librte_event_dlb2.a;-l:librte_event_dpaa.a;
                      -l:librte_event_dpaa2.a;-l:librte_event_dsw.a;-l:librte_event_opdl.a;-l:librte_event_skeleton.a;-l:librte_event_sw.a;
                      -l:librte_event_octeontx.a;-l:librte_baseband_acc.a;-l:librte_baseband_fpga_5gnr_fec.a;-l:librte_baseband_fpga_lte_fec.a;
                      -l:librte_baseband_la12xx.a;-l:librte_baseband_null.a;-l:librte_baseband_turbo_sw.a;-l:librte_node.a;-l:librte_graph.a;
                      -l:librte_pipeline.a;-l:librte_table.a;-l:librte_pdump.a;-l:librte_port.a;-l:librte_fib.a;-l:librte_pdcp.a;
                      -l:librte_ipsec.a;-l:librte_vhost.a;-l:librte_stack.a;-l:librte_security.a;-l:librte_sched.a;-l:librte_reorder.a;
                      -l:librte_rib.a;-l:librte_dmadev.a;-l:librte_mldev.a;-l:librte_regexdev.a;-l:librte_rawdev.a;-l:librte_power.a;
                      -l:librte_pcapng.a;-l:librte_member.a;-l:librte_lpm.a;-l:librte_latencystats.a;-l:librte_jobstats.a;-l:librte_ip_frag.a;
                      -l:librte_gso.a;-l:librte_gro.a;-l:librte_gpudev.a;-l:librte_eventdev.a;-l:librte_efd.a;-l:librte_distributor.a;
                      -l:librte_cryptodev.a;-l:librte_compressdev.a;-l:librte_cfgfile.a;-l:librte_bpf.a;-l:librte_bitratestats.a;
                      -l:librte_bbdev.a;-l:librte_acl.a;-l:librte_timer.a;-l:librte_hash.a;-l:librte_metrics.a;-l:librte_cmdline.a;
                      -l:librte_pci.a;-l:librte_ethdev.a;-l:librte_meter.a;-l:librte_net.a;-l:librte_mbuf.a;-l:librte_mempool.a;
                      -l:librte_rcu.a;-l:librte_ring.a;-l:librte_eal.a;-l:librte_telemetry.a;-l:librte_kvargs.a;
                      -Wl,--no-whole-archive;-Wl,--export-dynamic; -lglib-2.0 -lrt -pthread -lm -ldl -lnuma -lpcap)
