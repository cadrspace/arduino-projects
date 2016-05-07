#include <Arduino.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "WiFiUdp.h"
#include "utility/debug.h"
#include "utility/socket.h"
#include "UDPServer.h"
#include "coap.h"


/* CoAP */

#define PORT 5683
#define BUFSZ 256

uint8_t packetbuf[BUFSZ];
static uint8_t scratch_raw[32];
static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};


// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS,
					 ADAFRUIT_CC3000_IRQ,
					 ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER);

#define WLAN_SSID       "cadr"   // cannot be longer than 32 characters!
#define WLAN_PASS       "socketswitch"
#define WLAN_SECURITY   WLAN_SEC_WPA2


const int LED_START = 40;
const int LED_END   = 45;

UDPServer udp = UDPServer(PORT);


void displayMACAddress(void)
{
  uint8_t macAddress[6];

  if (! cc3000.getMacAddress(macAddress))
    {
      Serial.println(F("Unable to retrieve MAC Address!\r\n"));
      return;
    }

  Serial.print(F("MAC Address : "));
  cc3000.printHex((byte*)&macAddress, 6);
}

void wifi_init()
{
  Serial.println(F("\nInitializing..."));
  if (! cc3000.begin())
    {
      Serial.println(F("Couldn't begin()! Check your wiring?"));
      while (1);
    }

  displayMACAddress();
}

// Tries to read the IP address and other connection details
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if(! cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
    {
      Serial.println(F("Unable to retrieve the IP Address!\r\n"));
      return false;
    }
  else
    {
      Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
      Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
      Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
      Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
      Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
      Serial.println();
      return true;
    }
}


void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("smart-lights (server)\n")); 

  // Initialise the module

  wifi_init();
  /* cc3000.setDHCP(); */

  Serial.print(F("\nAttempting to connect to "));
  Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }

  Serial.println(F("Connected!"));

  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
    delay(100); // ToDo: Insert a DHCP timeout!

  // Display the IP address DNS, Gateway, etc.
  while (! displayConnectionDetails())
    delay(1000);

  udp.begin();
  endpoint_setup();

  Serial.println(F("Listening for connections..."));
}


void loop(void) {
  int sz;
  int rc;
  coap_packet_t pkt;
  if (udp.available()) {
    sz = udp.readData(packetbuf, BUFSZ);
    if (! sz)
      return;

    for (int i = 0; i < sz; i++) {
      Serial.print(packetbuf[i], HEX);
      Serial.print(" ");
    }

    if (0 != (rc = coap_parse(&pkt, packetbuf, sz))) {
      Serial.print("Bad packet rc=");
      Serial.println(rc, DEC);
    } else {
      size_t rsplen = sizeof(packetbuf);
      coap_packet_t rsppkt;
      coap_handle_req(&scratch_buf, &pkt, &rsppkt);

      memset(packetbuf, 0, UDP_TX_PACKET_MAX_SIZE);
      if (0 != (rc = coap_build(packetbuf, &rsplen, &rsppkt))) {
        Serial.print("coap_build failed rc=");
        Serial.println(rc, DEC);
      } else {
        int n = udp.sendData(packetbuf, rsplen);
        Serial.print(F("Size: "));
        Serial.println(n);
      }
    }
  }
}
