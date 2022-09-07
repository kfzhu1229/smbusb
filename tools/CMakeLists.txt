include(GNUInstallDirs)

add_executable(smbusb_comm smbusb_comm.c)
target_link_libraries(smbusb_comm smbusb::smbusb)

add_executable(smbusb_bq8030flasher smbusb_bq8030flasher.c)
target_link_libraries(smbusb_bq8030flasher smbusb::smbusb)

add_executable(smbusb_m37512flasher smbusb_m37512flasher.c)
target_link_libraries(smbusb_m37512flasher smbusb::smbusb)

add_executable(smbusb_r2j240flasher smbusb_r2j240flasher.c)
target_link_libraries(smbusb_r2j240flasher smbusb::smbusb)

add_executable(smbusb_sbsreport smbusb_sbsreport.c)
target_link_libraries(smbusb_sbsreport smbusb::smbusb)

add_executable(smbusb_scan smbusb_scan.c)
target_link_libraries(smbusb_scan smbusb::smbusb)


# Install

if (SMBUSB_NOT_SUBPROJECT)
    install(TARGETS
                smbusb_comm
                smbusb_bq8030flasher
                smbusb_m37512flasher
                smbusb_r2j240flasher
                smbusb_sbsreport
                smbusb_scan
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

