#include <Arduino.h>
#include <stdbool.h>
#include <string.h>
#include <USBAPI.h>
#include "coap.h"
#include "endpoints.h"

#ifdef DEBUG
#  define DEBUG(msg) Serial.print(msg)
#else
#  define DEBUG(msg) ((void) 0)
#endif /* ifdef DEBUG */


const int LED_START = 40;
const int LED_END   = 45;
unsigned char ledIDs[] = { 22, 24, 26, 28, 30, 32, 34, 36 };


enum { BUFSZ = 3 };
static char light[BUFSZ] = { 0 };
static size_t sz = 0;

const uint16_t rsplen = 1500;
static char rsp[1500] = "";


void build_rsp(void)
{
  uint16_t len = rsplen;
  const coap_endpoint_t *ep = endpoints;
  int i;

  len--; // Null-terminated string

  while (NULL != ep->handler)
    {
      if (NULL == ep->core_attr) {
        ep++;
        continue;
      }

      if (0 < strlen(rsp)) {
        strncat(rsp, ",", len);
        len--;
      }

      strncat(rsp, "<", len);
      len--;

      for (i = 0; i < ep->path->count; i++) {
        strncat(rsp, "/", len);
        len--;

        strncat(rsp, ep->path->elems[i], len);
        len -= strlen(ep->path->elems[i]);
      }

      strncat(rsp, ">;", len);
      len -= 2;

      strncat(rsp, ep->core_attr, len);
      len -= strlen(ep->core_attr);

      ep++;
    }
}

void setupLEDs() {
  for (int idx = 0; idx < sizeof(ledIDs); ++idx)
    pinMode(ledIDs[idx], OUTPUT);
}

void endpoint_setup(void)
{
  setupLEDs();
  build_rsp();
}


void on(int pin) {
   digitalWrite(pin, HIGH);
}

void off(int pin) {
  digitalWrite(pin, LOW);
}

void resetLEDs() {
  for (int pin = LED_START; pin <= LED_END; ++pin)
    off(pin);
}

bool enableLEDs(int leds) {
  resetLEDs();
  for (int idx = 0; idx < sizeof(ledIDs); ++idx) {
    DEBUG(F("LED: "));
    DEBUG(LED_START + idx);
    DEBUG("\n");
    if (leds & (1 << idx)) {
      on(ledIDs[idx]);
    } else {
      off(ledIDs[idx]);
    }
  }
  return true;
}


int stringToNumber(const uint8_t* buf, size_t sz) {
  int result = 0;
  for (int idx = sz - 1, dec = 1; idx >= 0; --idx, dec *= 10)
    result += (buf[idx] - 48) * dec;

  DEBUG(F("number = "));
  DEBUG(result);
  DEBUG("\n");

  return result;
}


static const coap_endpoint_path_t path_well_known_core = {
  2, {".well-known", "core"}
};

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
                                      const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt,
                                      uint8_t id_hi,
                                      uint8_t id_lo)
{
  return coap_make_response(scratch,
                            outpkt,
                            (const uint8_t *) rsp,
                            strlen(rsp),
                            id_hi,
                            id_lo,
                            &inpkt->tok,
                            COAP_RSPCODE_CONTENT,
                            COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}


static const coap_endpoint_path_t path_light = {1, {"light"}};

static int handle_get_light(coap_rw_buffer_t *scratch,
                            const coap_packet_t *inpkt,
                            coap_packet_t *outpkt,
                            uint8_t id_hi,
                            uint8_t id_lo)
{
  return coap_make_response(scratch,
                            outpkt,
                            (const uint8_t *) light,
                            strlen(light),
                            id_hi,
                            id_lo,
                            &inpkt->tok,
                            COAP_RSPCODE_CONTENT,
                            COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_light(coap_rw_buffer_t *scratch,
                            const coap_packet_t *inpkt,
                            coap_packet_t *outpkt,
                            uint8_t id_hi,
                            uint8_t id_lo)
{
  if ((inpkt->payload.len == 0) || (inpkt->payload.len > 3)) {
    return coap_make_response(scratch,
                              outpkt,
                              NULL,
                              0,
                              id_hi,
                              id_lo,
                              &inpkt->tok,
                              COAP_RSPCODE_BAD_REQUEST,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
  }

  memset (light, 0, BUFSZ);
  memcpy (light, inpkt->payload.p, inpkt->payload.len);

  int leds = stringToNumber(inpkt->payload.p,
                            inpkt->payload.len);

  enableLEDs(leds);

  return coap_make_response(scratch,
                            outpkt,
                            (const uint8_t *) &light,
                            inpkt->payload.len,
                            id_hi,
                            id_lo,
                            &inpkt->tok,
                            COAP_RSPCODE_CHANGED,
                            COAP_CONTENTTYPE_TEXT_PLAIN);
}


const coap_endpoint_t endpoints[] =
  {
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core, "ct=40"},
    {COAP_METHOD_GET, handle_get_light, &path_light, "ct=0"},
    {COAP_METHOD_PUT, handle_put_light, &path_light, NULL},
    {(coap_method_t)0, NULL, NULL, NULL}
  };

/* endpoints.cpp ends here. */
