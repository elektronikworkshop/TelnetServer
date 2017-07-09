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

#include "TelnetServer.h"

TelnetStreamProxy::TelnetStreamProxy(Stream& stream,
                                     bool doEcho)
  : m_stream(stream)
  , m_negotiationSequenceIndex(0)
  , m_lastWasCarriageReturn(false)
  , m_doEcho(doEcho)
{ }

size_t
TelnetStreamProxy::write(uint8_t c)
{
  size_t ret = 0;

  if ((char)c == '\n' and not m_lastWasCarriageReturn) {
    ret += m_stream.write('\r');
  }

  m_lastWasCarriageReturn = (char)c == '\r';

  ret += m_stream.write(c);
  return ret;
}

int
TelnetStreamProxy::available()
{
  return m_stream.available();
}

int
TelnetStreamProxy::read()
{
  while (true) {

    int d = m_stream.read();
    char c = d;

    if (d == -1) {
      return d;
    }

    /* echo it back (must be negotiated most probably) */
    if (m_doEcho) {
      m_stream.write(d);
    }

    /* strip negotiation sequences */
    if (c == 0xFF and m_negotiationSequenceIndex == 0) {
      m_negotiationSequenceIndex = 1;
      continue;
    }
    if (m_negotiationSequenceIndex) {
      if (m_negotiationSequenceIndex == 2) {
        m_negotiationSequenceIndex = 0;
      } else {
        m_negotiationSequenceIndex++;
      }
      continue;
    }

    /* strip other unwanted stuff here */

    return d;
  }
}

int
TelnetStreamProxy::peek()
{
  return m_stream.peek();
}

void
TelnetStreamProxy::flush()
{
  m_negotiationSequenceIndex = 0;
  m_lastWasCarriageReturn = false;
  m_stream.flush();
}

TelnetClient::TelnetClient(bool doEcho)
  : m_stream(m_client, doEcho)
  { }

void
TelnetClient::begin(const WiFiClient& client)
{
  if (m_connected) {
    return;
  }

  m_client = client;
  m_connected = true;
}

void
TelnetClient::reset()
{
  if (not m_connected) {
    return;
  }

  /* Also flushes WiFiClient */
  m_stream.flush();
  m_client.stop();

  m_connected = false;
}

void
TelnetClient::run()
{
  if (not m_client) {
    if (m_connected) {
      reset();
    }
    return;
  }

  if (m_client.connected()) {
    if (m_client.available()) {

      /* As long as client data is avaible we stream it through the telnet
       * stream proxy. It is then available from getStream().read() from
       * within any inherited class.
       */
      while (m_client.available()) {
        processStreamData();
      }
    }
  } else {
    reset();
  }
}

TelnetServer::TelnetServer(TelnetClient* clients,
                           size_t numClients,
                           uint16_t port)
  : m_server(port)
  , m_clients(clients)
  , m_numClients(numClients)
  , m_enabled(true)
{ }

void
TelnetServer::run()
{
  if (m_enabled) {
    if (m_server.status() == CLOSED) {
      m_server.begin();
      m_server.setNoDelay(true);
    }
  } else {
    if (m_server.status() != CLOSED) {

      /* stop clients */
      for (uint8_t i = 0; i < m_numClients; i++) {
        m_clients[i].reset();
      }

      m_server.stop();
    }
    return;
  }

  /* Handle new client connections */
  if (m_server.hasClient()) {

    /* Find free client object */
    bool handled = false;
    for (uint8_t i = 0; i < m_numClients; i++) {
      if (not m_clients[i].isConnected()) {
        m_clients[i].begin(m_server.available());
        handled = true;
      }
    }

    /* Any unhandled client connections get dumped here ("server rejected ...") */
    if (not handled) {
      WiFiClient client = m_server.available();
//        Debug << "dropping client connection from " << client.remoteIP().toString() << " -- no free client slot\n";
      client.stop();
    }
  }

  /* Process client connections */
  for (uint8_t i = 0; i < m_numClients; i++) {
    m_clients[i].run();
  }
}
