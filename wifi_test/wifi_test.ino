#include "wifi_autoconnect.h"

void setup() {
  setup_wifi("h01");
}

void loop() {
  handle_wifi();
}
