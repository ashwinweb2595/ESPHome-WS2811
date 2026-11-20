// Auto generated code by esphome
// ========== AUTO GENERATED INCLUDE BLOCK BEGIN ===========
#include "esphome.h"
using namespace esphome;
alignas(logger::Logger) static unsigned char logger__logger_logger_id__pstorage[sizeof(logger::Logger)];
static logger::Logger *const logger_logger_id = reinterpret_cast<logger::Logger *>(logger__logger_logger_id__pstorage);
#ifndef __PICOLIBC__
using std::isnan;
#endif
using std::min;
using std::max;
#include <new>
using namespace light;
alignas(web_server_base::WebServerBase) static unsigned char web_server_base__web_server_base_webserverbase_id__pstorage[sizeof(web_server_base::WebServerBase)];
static web_server_base::WebServerBase *const web_server_base_webserverbase_id = reinterpret_cast<web_server_base::WebServerBase *>(web_server_base__web_server_base_webserverbase_id__pstorage);
alignas(captive_portal::CaptivePortal) static unsigned char captive_portal__captive_portal_captiveportal_id__pstorage[sizeof(captive_portal::CaptivePortal)];
static captive_portal::CaptivePortal *const captive_portal_captiveportal_id = reinterpret_cast<captive_portal::CaptivePortal *>(captive_portal__captive_portal_captiveportal_id__pstorage);
alignas(wifi::WiFiComponent) static unsigned char wifi__wifi_wificomponent_id__pstorage[sizeof(wifi::WiFiComponent)];
static wifi::WiFiComponent *const wifi_wificomponent_id = reinterpret_cast<wifi::WiFiComponent *>(wifi__wifi_wificomponent_id__pstorage);
alignas(mdns::MDNSComponent) static unsigned char mdns__mdns_mdnscomponent_id__pstorage[sizeof(mdns::MDNSComponent)];
static mdns::MDNSComponent *const mdns_mdnscomponent_id = reinterpret_cast<mdns::MDNSComponent *>(mdns__mdns_mdnscomponent_id__pstorage);
alignas(network::NetworkComponent) static unsigned char network__network_networkcomponent_id__pstorage[sizeof(network::NetworkComponent)];
static network::NetworkComponent *const network_networkcomponent_id = reinterpret_cast<network::NetworkComponent *>(network__network_networkcomponent_id__pstorage);
alignas(esphome::ESPHomeOTAComponent) static unsigned char esphome__esphome_esphomeotacomponent_id__pstorage[sizeof(esphome::ESPHomeOTAComponent)];
static esphome::ESPHomeOTAComponent *const esphome_esphomeotacomponent_id = reinterpret_cast<esphome::ESPHomeOTAComponent *>(esphome__esphome_esphomeotacomponent_id__pstorage);
alignas(web_server::WebServerOTAComponent) static unsigned char web_server__web_server_webserverotacomponent_id__pstorage[sizeof(web_server::WebServerOTAComponent)];
static web_server::WebServerOTAComponent *const web_server_webserverotacomponent_id = reinterpret_cast<web_server::WebServerOTAComponent *>(web_server__web_server_webserverotacomponent_id__pstorage);
alignas(preferences::IntervalSyncer) static unsigned char preferences__preferences_intervalsyncer_id__pstorage[sizeof(preferences::IntervalSyncer)];
static preferences::IntervalSyncer *const preferences_intervalsyncer_id = reinterpret_cast<preferences::IntervalSyncer *>(preferences__preferences_intervalsyncer_id__pstorage);
alignas(safe_mode::SafeModeComponent) static unsigned char safe_mode__safe_mode_safemodecomponent_id__pstorage[sizeof(safe_mode::SafeModeComponent)];
static safe_mode::SafeModeComponent *const safe_mode_safemodecomponent_id = reinterpret_cast<safe_mode::SafeModeComponent *>(safe_mode__safe_mode_safemodecomponent_id__pstorage);
alignas(api::APIServer) static unsigned char api__api_apiserver_id__pstorage[sizeof(api::APIServer)];
static api::APIServer *const api_apiserver_id = reinterpret_cast<api::APIServer *>(api__api_apiserver_id__pstorage);
using namespace api;
alignas(web_server::WebServer) static unsigned char web_server__web_server_webserver_id__pstorage[sizeof(web_server::WebServer)];
static web_server::WebServer *const web_server_webserver_id = reinterpret_cast<web_server::WebServer *>(web_server__web_server_webserver_id__pstorage);
constexpr uint8_t ESPHOME_WEBSERVER_INDEX_HTML[174] PROGMEM = {60, 33, 68, 79, 67, 84, 89, 80, 69, 32, 104, 116, 109, 108, 62, 60, 104, 116, 109, 108, 62, 60, 104, 101, 97, 100, 62, 60, 109, 101, 116, 97, 32, 99, 104, 97, 114, 115, 101, 116, 61, 85, 84, 70, 45, 56, 62, 60, 108, 105, 110, 107, 32, 114, 101, 108, 61, 105, 99, 111, 110, 32, 104, 114, 101, 102, 61, 100, 97, 116, 97, 58, 62, 60, 47, 104, 101, 97, 100, 62, 60, 98, 111, 100, 121, 62, 60, 101, 115, 112, 45, 97, 112, 112, 62, 60, 47, 101, 115, 112, 45, 97, 112, 112, 62, 60, 115, 99, 114, 105, 112, 116, 32, 115, 114, 99, 61, 34, 104, 116, 116, 112, 115, 58, 47, 47, 111, 105, 46, 101, 115, 112, 104, 111, 109, 101, 46, 105, 111, 47, 118, 50, 47, 119, 119, 119, 46, 106, 115, 34, 62, 60, 47, 115, 99, 114, 105, 112, 116, 62, 60, 47, 98, 111, 100, 121, 62, 60, 47, 104, 116, 109, 108, 62};
constexpr size_t ESPHOME_WEBSERVER_INDEX_HTML_SIZE = 174;
using namespace json;
alignas(esp32_rmt_led_strip::ESP32RMTLEDStripLightOutput) static unsigned char esp32_rmt_led_strip__esp32_rmt_led_strip_esp32rmtledstriplightoutput_id__pstorage[sizeof(esp32_rmt_led_strip::ESP32RMTLEDStripLightOutput)];
static esp32_rmt_led_strip::ESP32RMTLEDStripLightOutput *const esp32_rmt_led_strip_esp32rmtledstriplightoutput_id = reinterpret_cast<esp32_rmt_led_strip::ESP32RMTLEDStripLightOutput *>(esp32_rmt_led_strip__esp32_rmt_led_strip_esp32rmtledstriplightoutput_id__pstorage);
alignas(light::AddressableLightState) static unsigned char light__led_strip__pstorage[sizeof(light::AddressableLightState)];
static light::AddressableLightState *const led_strip = reinterpret_cast<light::AddressableLightState *>(light__led_strip__pstorage);
#undef yield
#define yield() esphome::yield()
#undef millis
#define millis() esphome::millis()
#undef micros
#define micros() esphome::micros()
#undef delay
#define delay(x) esphome::delay(x)
#undef delayMicroseconds
#define delayMicroseconds(x) esphome::delayMicroseconds(x)
#include <tuple>
static auto esphome_controllers() { return std::tuple{api_apiserver_id, web_server_webserver_id}; }
#include "esphome/core/controller_dispatch.h"
static constexpr size_t ESPHOME_LOOPING_COMPONENT_COUNT = \
  (1 * HasLoopOverride<logger::Logger>::value) + \
  (1 * HasLoopOverride<captive_portal::CaptivePortal>::value) + \
  (1 * HasLoopOverride<wifi::WiFiComponent>::value) + \
  (1 * HasLoopOverride<mdns::MDNSComponent>::value) + \
  (1 * HasLoopOverride<network::NetworkComponent>::value) + \
  (1 * HasLoopOverride<esphome::ESPHomeOTAComponent>::value) + \
  (1 * HasLoopOverride<preferences::IntervalSyncer>::value) + \
  (1 * HasLoopOverride<safe_mode::SafeModeComponent>::value) + \
  (1 * HasLoopOverride<web_server::WebServerOTAComponent>::value) + \
  (1 * HasLoopOverride<api::APIServer>::value) + \
  (1 * HasLoopOverride<web_server::WebServer>::value) + \
  (1 * HasLoopOverride<light::AddressableLightState>::value) + \
  (1 * HasLoopOverride<esp32_rmt_led_strip::ESP32RMTLEDStripLightOutput>::value);
namespace esphome {
static const char COMP_SRC_TABLE_STR_0[] PROGMEM = "logger";
static const char COMP_SRC_TABLE_STR_1[] PROGMEM = "captive_portal";
static const char COMP_SRC_TABLE_STR_2[] PROGMEM = "wifi";
static const char COMP_SRC_TABLE_STR_3[] PROGMEM = "mdns";
static const char COMP_SRC_TABLE_STR_4[] PROGMEM = "network";
static const char COMP_SRC_TABLE_STR_5[] PROGMEM = "esphome.ota";
static const char COMP_SRC_TABLE_STR_6[] PROGMEM = "preferences";
static const char COMP_SRC_TABLE_STR_7[] PROGMEM = "safe_mode";
static const char COMP_SRC_TABLE_STR_8[] PROGMEM = "web_server.ota";
static const char COMP_SRC_TABLE_STR_9[] PROGMEM = "api";
static const char COMP_SRC_TABLE_STR_10[] PROGMEM = "web_server";
static const char COMP_SRC_TABLE_STR_11[] PROGMEM = "light";
static const char COMP_SRC_TABLE_STR_12[] PROGMEM = "esp32_rmt_led_strip.light";
static const char *const COMP_SRC_TABLE[] PROGMEM = {COMP_SRC_TABLE_STR_0, COMP_SRC_TABLE_STR_1, COMP_SRC_TABLE_STR_2, COMP_SRC_TABLE_STR_3, COMP_SRC_TABLE_STR_4, COMP_SRC_TABLE_STR_5, COMP_SRC_TABLE_STR_6, COMP_SRC_TABLE_STR_7, COMP_SRC_TABLE_STR_8, COMP_SRC_TABLE_STR_9, COMP_SRC_TABLE_STR_10, COMP_SRC_TABLE_STR_11, COMP_SRC_TABLE_STR_12};
const LogString *component_source_lookup(uint8_t index) {
  if (index == 0 || index > 13) return LOG_STR("<unknown>");
  return reinterpret_cast<const LogString *>(
    progmem_read_ptr(&COMP_SRC_TABLE[index - 1]));
}
}  // namespace esphome
static constexpr uint8_t noise_psk[] PROGMEM = {219, 74, 167, 73, 5, 130, 242, 64, 1, 137, 29, 176, 230, 19, 10, 242, 78, 207, 63, 51, 117, 21, 107, 53, 175, 122, 34, 54, 127, 209, 149, 45};
// ========== AUTO GENERATED INCLUDE BLOCK END ==========="

void setup() {
  // ========== AUTO GENERATED CODE BEGIN ===========
  // logger:
  //   baud_rate: 115200
  //   deassert_rts_dtr: false
  //   hardware_uart: UART0
  //   id: logger_logger_id
  //   level: DEBUG
  //   logs: {}
  //   runtime_tag_levels: false
  //   task_log_buffer_size: 768
  //   tx_buffer_size: 512
  new(logger_logger_id) logger::Logger(115200);
  logger_logger_id->create_pthread_key();
  logger_logger_id->pre_setup();
  logger_logger_id->set_log_level(ESPHOME_LOG_LEVEL_DEBUG);
  // network:
  //   enable_ipv6: false
  //   id: network_networkcomponent_id
  //   min_ipv6_addr_count: 0
  // esphome:
  //   areas: []
  //   build_flags: []
  //   build_path: build\smart-led-controller
  //   debug_scheduler: false
  //   devices: []
  //   environment_variables: {}
  //   friendly_name: Smart LED Controller
  //   includes: []
  //   includes_c: []
  //   libraries: []
  //   merge_warnings: true
  //   min_version: 2026.10.2
  //   name: smart-led-controller
  //   name_add_mac_suffix: false
  //   platformio_options: {}
  //   suspend_loop: false
  new (&App) Application;
  App.pre_setup("smart-led-controller", 20, "Smart LED Controller", 20);
  App.looping_components_.init(ESPHOME_LOOPING_COMPONENT_COUNT);
  // light:
  App.register_component_(logger_logger_id, 1);
  // web_server_base:
  //   id: web_server_base_webserverbase_id
  new(web_server_base_webserverbase_id) web_server_base::WebServerBase();
  web_server_base::global_web_server_base = web_server_base_webserverbase_id;
  // captive_portal:
  //   compression: gzip
  //   id: captive_portal_captiveportal_id
  //   web_server_base_id: web_server_base_webserverbase_id
  new(captive_portal_captiveportal_id) captive_portal::CaptivePortal(web_server_base_webserverbase_id);
  App.register_component_(captive_portal_captiveportal_id, 2);
  // wifi:
  //   ap:
  //     ap_timeout: 90s
  //     id: wifi_wifiap_id
  //     password: !secret 'fallback_password'
  //     ssid: \033[8mSmart-LED-Fallback\033[28m
  //   domain: .local
  //   enable_btm: false
  //   enable_on_boot: true
  //   enable_rrm: false
  //   fast_connect:
  //     enabled: false
  //     storage: flash
  //   id: wifi_wificomponent_id
  //   min_auth_mode: WPA2
  //   networks:
  //     - id: wifi_wifiap_id_2
  //       password: !secret 'wifi_password'
  //       priority: 0
  //       ssid: !secret 'wifi_ssid'
  //   passive_scan: false
  //   post_connect_roaming: true
  //   power_save_mode: LIGHT
  //   reboot_timeout: 15min
  //   use_address: smart-led-controller.local
  new(wifi_wificomponent_id) wifi::WiFiComponent();
  wifi_wificomponent_id->init_sta(1);
  {
  wifi::WiFiAP wifi_wifiap_id_2 = wifi::WiFiAP();
  wifi_wifiap_id_2.set_ssid("Ashwin_Test");
  wifi_wifiap_id_2.set_password("Ashwin12345");
  wifi_wificomponent_id->add_sta(wifi_wifiap_id_2);
  }
  {
  wifi::WiFiAP wifi_wifiap_id = wifi::WiFiAP();
  wifi_wifiap_id.set_ssid("Smart-LED-Fallback");
  wifi_wifiap_id.set_password("ChangeThisPassword123");
  wifi_wificomponent_id->set_ap(wifi_wifiap_id);
  }
  wifi_wificomponent_id->set_power_save_mode(wifi::WIFI_POWER_SAVE_LIGHT);
  App.register_component_(wifi_wificomponent_id, 3);
  // mdns:
  //   disabled: false
  //   id: mdns_mdnscomponent_id
  //   services: []
  new(mdns_mdnscomponent_id) mdns::MDNSComponent();
  App.register_component_(mdns_mdnscomponent_id, 4);
  new(network_networkcomponent_id) network::NetworkComponent();
  App.register_component_(network_networkcomponent_id, 5);
  // ota:
  // ota.esphome:
  //   allow_partition_access: false
  //   id: esphome_esphomeotacomponent_id
  //   password: !secret 'ota_password'
  //   platform: esphome
  //   port: 3232
  //   version: 2
  new(esphome_esphomeotacomponent_id) esphome::ESPHomeOTAComponent();
  esphome_esphomeotacomponent_id->set_port(3232);
  esphome_esphomeotacomponent_id->set_auth_password("Your_Strong_OTA_Password");
  esphome_esphomeotacomponent_id->set_noise_psk(noise_psk);
  App.register_component_(esphome_esphomeotacomponent_id, 6);
  // ota.web_server:
  //   id: web_server_webserverotacomponent_id
  //   platform: web_server
  new(web_server_webserverotacomponent_id) web_server::WebServerOTAComponent();
  // preferences:
  //   flash_write_interval: 60s
  //   id: preferences_intervalsyncer_id
  new(preferences_intervalsyncer_id) preferences::IntervalSyncer();
  preferences_intervalsyncer_id->set_update_interval(60000);
  App.register_component_(preferences_intervalsyncer_id, 7);
  // safe_mode:
  //   boot_is_good_after: 1min
  //   boot_is_good_on_shutdown: true
  //   disabled: false
  //   id: safe_mode_safemodecomponent_id
  //   num_attempts: 10
  //   reboot_timeout: 5min
  //   storage: flash
  new(safe_mode_safemodecomponent_id) safe_mode::SafeModeComponent();
  App.register_component_(safe_mode_safemodecomponent_id, 8);
  if (safe_mode_safemodecomponent_id->should_enter_safe_mode(10, 300000, 60000, true)) return;
  App.register_component_(web_server_webserverotacomponent_id, 9);
  // api:
  //   batch_delay: 100ms
  //   custom_services: false
  //   encryption:
  //     key: !secret 'api_encryption_key'
  //   homeassistant_services: false
  //   homeassistant_states: false
  //   id: api_apiserver_id
  //   listen_backlog: 4
  //   max_connections: 5
  //   max_send_queue: 8
  //   port: 6053
  //   reboot_timeout: 15min
  new(api_apiserver_id) api::APIServer();
  App.register_component_(api_apiserver_id, 10);
  api_apiserver_id->set_noise_psk(noise_psk);
  // web_server:
  //   compression: gzip
  //   css_url: ''
  //   enable_private_network_access: false
  //   id: web_server_webserver_id
  //   include_internal: false
  //   js_url: https:oi.esphome.io/v2/www.js
  //   log: true
  //   port: 80
  //   version: 2
  //   web_server_base_id: web_server_base_webserverbase_id
  new(web_server_webserver_id) web_server::WebServer(web_server_base_webserverbase_id);
  App.register_component_(web_server_webserver_id, 11);
  // json:
  //   {}
  // esp32:
  //   board: esp32dev
  //   cpu_frequency: 240MHZ
  //   flash_size: 4MB
  //   framework:
  //     advanced:
  //       adc_oneshot_in_iram: false
  //       compiler_optimization: SIZE
  //       disable_debug_stubs: true
  //       disable_dev_null_vfs: true
  //       disable_fatfs: true
  //       disable_libc_locks_in_iram: true
  //       disable_mbedtls_peer_cert: true
  //       disable_mbedtls_pkcs7: true
  //       disable_mbedtls_tls: true
  //       disable_mbedtls_tls_extras: true
  //       disable_mbedtls_tls_server: true
  //       disable_ocd_aware: true
  //       disable_regi2c_in_iram: true
  //       disable_usb_serial_jtag_secondary: true
  //       disable_vfs_support_dir: true
  //       disable_vfs_support_select: true
  //       disable_vfs_support_termios: true
  //       enable_full_printf: false
  //       enable_idf_experimental_features: false
  //       enable_lwip_assert: true
  //       enable_lwip_bridge_interface: false
  //       enable_lwip_check_thread_safety: true
  //       enable_lwip_mdns_queries: true
  //       enable_lwip_tcpip_core_locking: true
  //       enable_ota_downgrade_protection: false
  //       enable_ota_rollback: true
  //       execute_from_psram: false
  //       freertos_in_iram: false
  //       heap_in_iram: false
  //       ignore_efuse_custom_mac: false
  //       ignore_efuse_mac_crc: false
  //       include_builtin_idf_components: []
  //       loop_task_stack_size: 8192
  //       ringbuf_in_iram: false
  //       sram1_as_iram: false
  //       use_full_certificate_bundle: false
  //     components: []
  //     log_level: ERROR
  //     sdkconfig_options: {}
  //     type: arduino
  //     version: 3.3.12
  //   variant: ESP32
  //   watchdog_timeout: 5s
  // light.esp32_rmt_led_strip:
  //   channel_colors: GRB
  //   chipset: WS2811
  //   default_transition_length: 1500ms
  //   disabled_by_default: false
  //   flash_transition_length: 0s
  //   gamma_correct: 2.8
  //   gamma_table_id: light_gammatable_id
  //   id: led_strip
  //   name: WS2811 LED Strip
  //   num_leds: 66
  //   output_id: esp32_rmt_led_strip_esp32rmtledstriplightoutput_id
  //   pin:
  //     drive_strength: 20.0
  //     id: esp32_esp32internalgpiopin_id
  //     ignore_pin_validation_error: false
  //     ignore_strapping_warning: false
  //     inverted: false
  //     mode:
  //       input: false
  //       open_drain: false
  //       output: true
  //       pulldown: false
  //       pullup: false
  //     number: 23
  //   platform: esp32_rmt_led_strip
  //   reset_high: 0us
  //   reset_low: 0us
  //   restore_mode: RESTORE_DEFAULT_OFF
  //   resume_effect: false
  //   rmt_symbols: 192
  //   use_psram: true
  new(esp32_rmt_led_strip_esp32rmtledstriplightoutput_id) esp32_rmt_led_strip::ESP32RMTLEDStripLightOutput();
  new(led_strip) light::AddressableLightState(esp32_rmt_led_strip_esp32rmtledstriplightoutput_id);
  App.register_component_(led_strip, 12);
  led_strip->set_save_enabled(true);
  led_strip->set_default_transition_length(1500);
  static constexpr light::GammaTable light_gammatable_id PROGMEM = {{0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0003, 0x0004, 0x0006, 0x0008, 0x000A, 0x000D, 0x0010, 0x0013, 0x0018, 0x001C, 0x0021, 0x0027, 0x002E, 0x0035, 0x003C, 0x0045, 0x004E, 0x0058, 0x0062, 0x006E, 0x007A, 0x0087, 0x0095, 0x00A4, 0x00B3, 0x00C4, 0x00D6, 0x00E8, 0x00FC, 0x0111, 0x0127, 0x013D, 0x0155, 0x016E, 0x0189, 0x01A4, 0x01C1, 0x01DE, 0x01FE, 0x021E, 0x023F, 0x0262, 0x0287, 0x02AC, 0x02D3, 0x02FC, 0x0326, 0x0351, 0x037E, 0x03AC, 0x03DC, 0x040D, 0x0440, 0x0474, 0x04AA, 0x04E2, 0x051B, 0x0556, 0x0593, 0x05D1, 0x0611, 0x0653, 0x0696, 0x06DC, 0x0723, 0x076C, 0x07B7, 0x0803, 0x0852, 0x08A2, 0x08F5, 0x0949, 0x099F, 0x09F8, 0x0A52, 0x0AAE, 0x0B0D, 0x0B6D, 0x0BD0, 0x0C34, 0x0C9B, 0x0D04, 0x0D6F, 0x0DDC, 0x0E4C, 0x0EBE, 0x0F32, 0x0FA8, 0x1020, 0x109B, 0x1118, 0x1198, 0x121A, 0x129E, 0x1325, 0x13AE, 0x1439, 0x14C7, 0x1558, 0x15EB, 0x1680, 0x1718, 0x17B3, 0x1850, 0x18F0, 0x1992, 0x1A37, 0x1ADF, 0x1B89, 0x1C36, 0x1CE5, 0x1D98, 0x1E4D, 0x1F05, 0x1FC0, 0x207D, 0x213D, 0x2200, 0x22C6, 0x238F, 0x245B, 0x252A, 0x25FB, 0x26D0, 0x27A7, 0x2882, 0x295F, 0x2A40, 0x2B23, 0x2C0A, 0x2CF3, 0x2DE0, 0x2ED0, 0x2FC3, 0x30B9, 0x31B2, 0x32AF, 0x33AE, 0x34B1, 0x35B7, 0x36C1, 0x37CD, 0x38DD, 0x39F1, 0x3B07, 0x3C21, 0x3D3E, 0x3E5F, 0x3F83, 0x40AA, 0x41D5, 0x4303, 0x4435, 0x456A, 0x46A3, 0x47DF, 0x491F, 0x4A62, 0x4BA9, 0x4CF4, 0x4E42, 0x4F94, 0x50E9, 0x5242, 0x539F, 0x54FF, 0x5663, 0x57CB, 0x5936, 0x5AA6, 0x5C19, 0x5D90, 0x5F0A, 0x6089, 0x620B, 0x6391, 0x651C, 0x66AA, 0x683B, 0x69D1, 0x6B6B, 0x6D09, 0x6EAA, 0x7050, 0x71FA, 0x73A8, 0x7559, 0x770F, 0x78C9, 0x7A87, 0x7C4A, 0x7E10, 0x7FDA, 0x81A9, 0x837C, 0x8553, 0x872E, 0x890D, 0x8AF1, 0x8CD9, 0x8EC5, 0x90B6, 0x92AB, 0x94A4, 0x96A1, 0x98A3, 0x9AA9, 0x9CB4, 0x9EC3, 0xA0D7, 0xA2EF, 0xA50B, 0xA72C, 0xA952, 0xAB7B, 0xADAA, 0xAFDD, 0xB214, 0xB451, 0xB691, 0xB8D7, 0xBB21, 0xBD6F, 0xBFC3, 0xC21B, 0xC477, 0xC6D9, 0xC93F, 0xCBAA, 0xCE19, 0xD08E, 0xD307, 0xD585, 0xD807, 0xDA8F, 0xDD1C, 0xDFAD, 0xE243, 0xE4DE, 0xE77E, 0xEA23, 0xECCD, 0xEF7C, 0xF230, 0xF4E9, 0xF7A7, 0xFA6A, 0xFD32, 0xFFFF}, 280};
  led_strip->set_gamma_table(&light_gammatable_id);
  App.register_light(led_strip, "WS2811 LED Strip", 1133278602, 0);
  App.register_component_(esp32_rmt_led_strip_esp32rmtledstriplightoutput_id, 13);
  esp32_rmt_led_strip_esp32rmtledstriplightoutput_id->set_num_leds(66);
  esp32_rmt_led_strip_esp32rmtledstriplightoutput_id->set_pin(23);
  esp32_rmt_led_strip_esp32rmtledstriplightoutput_id->set_led_params(300, 1090, 1090, 320, 0, 300000);
  esp32_rmt_led_strip_esp32rmtledstriplightoutput_id->set_channel_colors(light::ChannelColors{
      .r = 1,
      .g = 0,
      .b = 2,
      .w = light::ChannelColors::NO_WHITE,
  });
  esp32_rmt_led_strip_esp32rmtledstriplightoutput_id->set_use_psram(true);
  esp32_rmt_led_strip_esp32rmtledstriplightoutput_id->set_rmt_symbols(192);
  // md5:
  // socket:
  //   implementation: bsd_sockets
  // web_server_idf:
  //   {}
  // noise:
  //   {}
  // sha256:
  //   {}
  // =========== AUTO GENERATED CODE END ============
  App.setup();
}

void loop() {
  App.loop();
}
