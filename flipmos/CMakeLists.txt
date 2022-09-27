cmake_minimum_required(VERSION 3.16)
project("Flipmos")

set(SSID "osaa-5" CACHE STRING "The access point ssid to us")
set(PASSWORD "deadbeef" CACHE STRING "The wifi password ")
set(MDNS_NAME "flipmos" CACHE STRING "The multicast name")
set(WEMOS_PORT "/dev/usb0" CACHE STRING "The port where the wemos is attached")
set(FQBN "esp8266:esp8266:d1" CACHE STRING "The Fully Qualified Board Name")

configure_file(
  Config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.h
  @ONLY
  )

add_custom_target(
  flipmos_init
  #works on ubuntu something
#  COMMAND sudo snap install arduino-cli
  COMMAND arduino-cli --config-file arduino-cli.yaml core update-index
  COMMAND arduino-cli --config-file arduino-cli.yaml core install esp8266:esp8266
  COMMAND arduino-cli --config-file arduino-cli.yaml lib install "Adafruit GFX Library"
  COMMAND arduino-cli --config-file arduino-cli.yaml lib install "Adafruit SSD1306 Wemos Mini OLED"
)

set_target_properties(
  flipmos_init
  PROPERTIES
  EXCLUDE_FROM_ALL 1
)


add_custom_target(
  flipmos
  COMMAND arduino-cli --config-file arduino-cli.yaml compile --fqbn ${FQBN} ${CMAKE_CURRENT_SOURCE_DIR}/flipmos.ino
)

add_custom_target(
  flipmos_upload
  COMMAND arduino-cli --config-file arduino-cli.yaml upload -p ${WEMOS_PORT} --fqbn ${FQBN} ${CMAKE_CURRENT_SOURCE_DIR}/flipmos.ino

)

# Do not upload when building all
set_target_properties(
  flipmos_upload
  PROPERTIES
  EXCLUDE_FROM_ALL 1
)

