/** TelnetServer - A telnet server implementation for Arduino
 *
 * Copyright (C) 2017 Elektronik Workshop <hoi@elektronikworkshop.ch>
 * http://elektronikworkshop.ch
 *
 * See the README for more information or take a look at the code.
 *
 ***
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Further reading:
 *
 *  * http://mud-dev.wikidot.com/telnet:negotiation
 *
 */

#ifndef _EW_TELNET_SERVER_H_
#define _EW_TELNET_SERVER_H_

#if 0 //defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
  // SAM-specific code
# include <WiFi.h>
/* todo for non-ESP compilation, volunteers?:
/home/uli/sketchbook/libraries/TelnetServer/src/TelnetServer.h:204:32: error: 'CLOSED' was not declared in this scope
       if (m_server.status() == CLOSED) {                              ^
/home/uli/sketchbook/libraries/TelnetServer/src/TelnetServer.h:206:18: error: 'class WiFiServer' has no member named 'setNoDelay
*/
#elif defined(ARDUINO_ARCH_ESP8266)
# include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
# include <WiFi.h>
#else
# error “This library currenly only supports boards with an ESPxx processor”
#endif

/** A stream proxy which handles telnet logic (negotiation sequences)
 * and cleans up the stream for processing downstream.
 *
 */
class TelnetStreamProxy
  : public Stream
{
private:
  Stream& m_stream;
  uint8_t m_negotiationSequenceIndex;
  bool m_lastWasCarriageReturn;
  bool m_doEcho;

public:
  TelnetStreamProxy(Stream& stream, bool doEcho);

  /* Implementation of Stream/Print abstract functions */
  virtual size_t write(uint8_t c);
  virtual int available();
  virtual int read();
  virtual int peek();
  virtual void flush();
};

/**
 *
 */
class TelnetClient
{
public:
  TelnetClient(bool doEcho = false);

  virtual void begin(const WiFiClient& client);
  virtual void reset();
  virtual void run();
  bool isConnected()
  {
    return m_connected;
  }

protected:
  WiFiClient& getClient() { return m_client; }
  TelnetStreamProxy& getStream() { return m_stream; }
  virtual void processStreamData() = 0;

private:
  WiFiClient m_client;
  TelnetStreamProxy m_stream;
  bool m_connected;
};

/**
 *
 */
class TelnetServer
{
public:
  TelnetServer(TelnetClient* clients,
               size_t numClients,
               uint16_t port = 23);
  void enable(bool enable)
  {
    m_enabled = enable;
  }
  bool isEnabled() const
  {
    return m_enabled;
  }
  virtual void run();
private:
  WiFiServer m_server;
  TelnetClient* m_clients;
  size_t m_numClients;
  bool m_enabled;
};

#endif  /* _EW_TELNET_SERVER_H_ */
