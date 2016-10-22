set(SSID "osaa-5" CACHE STRING "The access point ssid to us")
set(PASSWORD "deadbeef" CACHE STRING "The wifi password ")
set(MDNS_NAME "flipmos" CACHE STRING "The multicast name")

configure_file(
  Config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.h
  @ONLY
  )

add_custom_target(
  flipmos
  COMMAND arduino --verify ${CMAKE_CURRENT_SOURCE_DIR}/flipmos.ino
)

add_custom_target(
  flipmos_upload
  COMMAND arduino --upload ${CMAKE_CURRENT_SOURCE_DIR}/flipmos.ino
)

# Do not upload when building all
set_target_properties(
  flipmos_upload
  PROPERTIES
  EXCLUDE_FROM_ALL 1
)
