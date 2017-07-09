/** TelnetPinManipulator, a TelnetServer example
 *
 * Copyright (C) 2017 Elektronik Workshop <hoi@elektronikworkshop.ch>
 * http://elektronikworkshop.ch
 *
 **
 *  This example demonstrates the use of the StreamCmd class and shows you
 *  how to build a simple but already pretty powerful command line interface
 *  which performs argument and option parsing with type and range checking.
 **
 *  This file is part of the TelnetServer library and covered by the GNU Lesser
 *  General Public License v3.0, see the LICENSE file for more details on the
 *  license.
 */

#if 0 //defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
  /* TODO -- volunteers ? */
#elif defined(ARDUINO_ARCH_ESP8266)
# include <ESP8266mDNS.h>
#else
# error “This library currenly only supports boards with an ESP8266 processor”
#endif

#include <StreamCmd.h>
#include <TelnetServer.h>

/* vvvvvvvvvvvvvvvvvvvvvvvvvvv user configuration vvvvvvvvvvvvvvvvvvvvvvvvvvv */
const char* WiFiSsid  = "<your ssid>";
const char* WiFiPass  = "<your pass>";

const size_t MaxTelnetClients = 1;

const char* HostName = "esp-telnet-cli";
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */


#define arraysize(x)  sizeof(x) / sizeof(x[0])
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

/**
 * This generic code should make it easy to adapt the pin mapping to any
 * platform, just edit dpins and apins - that's it.
 */
struct Pin {
  uint8_t i;
  const char* n;
};
const Pin dpins[] =
  {{D0, "d0"}, {D1, "d1"}, {D2, "d2"}, {D3, "d3"}, {D4, "d4"}, {D5, "d5"},
   {D6, "d6"}, {D7, "d7"}, {D8, "d8"}};
const Pin apins[] =
  {{A0, "a0"}};
const uint8_t ndpins = arraysize(dpins);
const uint8_t napins = arraysize(apins);
const uint8_t npins  = ndpins + napins;


class TelnetCli
  : public TelnetClient
  , public StreamCmd<2 /* number of command sets */>
{
public:
  typedef StreamCmd<2> CliBase;
  enum CommandSet
  {
    Authenticated = 0,
    NotAuthenticated,
  };
  TelnetCli()
    : CliBase(getStream(), '\r' /* eol */, HostName)
  {
    uint8_t p(0);
    for (size_t i(0); i < ndpins; i++, p++) {
      m_pinNames[p] = dpins[i].n;
    }
    for (size_t i(0); i < napins; i++, p++) {
      m_pinNames[p] = apins[i].n;
    }

    switchCommandSet(Authenticated);
    setDefaultHandler(&TelnetCli::def);
    addCommand("help", &TelnetCli::help);
    addCommand("quit", &TelnetCli::quit);
    addCommand("pin",  &TelnetCli::pin);

    switchCommandSet(NotAuthenticated);
    setDefaultHandler(&TelnetCli::auth);
  }
private:
  void auth(const char* password)
  {
    if (strcmp(password, "h4ckm3") == 0) {
      switchCommandSet(Authenticated);
      getStream() << "authentication successful, try \"help\"\n";
      return;
    }
    /* we're mean */
    reset();
  }
  void def(const char* command)
  {
    if (command and strlen(command)) {
      getStream() << "unknown command \"" << command << "\", try \"help\"\n";
    }
  }
  void help()
  {
    getStream() <<
      "help -- print this help\n"
      "quit -- terminate telnet session\n"
      "pin <pin> [op] -- perform operation on pin <pin>\n"
      "  <pin>  pin identifier one out of {d0 .. d8} and {a0}\n"
      "  [op]   operation, no operation reads pin.\n"
      "    The following write operations are available on\n"
      "    digital pins only:\n"
      "      h  set pin high\n"
      "      l  set pin low\n"
      "      0 .. 255  set PWM level\n"
      "\n"
      "<> : mandatory arguments\n"
      "[] : optional arguments\n"
      ;
  }
  void quit()
  {
    reset();
  }
  void pin()
  {
    size_t pinidx(0);
    switch (getOption(pinidx, m_pinNames, npins)) {
      case ArgOk:
        break;
      case ArgNone:
        getStream() << "no <pin> argument\n";
        return;
      default:
        getStream() << "invalid <pin> argument \"" << current() << "\", allowed: {";
        for (uint8_t i(0); i < npins; i++) {
          getStream() << m_pinNames[i] << (i < npins - 1 ? ", " : "");
        }
        getStream() << "}\n";
        return;
    }

    /* no argument: read
     * h : set high
     * l : set low
     * 0 .. 255 write analog
     */
    size_t opidx(0);
    enum {OpHigh = 0, OpLow};
    switch (getOpt(opidx, "h", "l")) {
      case ArgOk:
        if (pinidx >= ndpins) {
          getStream() << "you can only write digital pins, \"" << m_pinNames[pinidx] << "\" is an analog pin\n";
          return;
        }
        getStream() << "setting digital pin \"" << m_pinNames[pinidx] << "\" to \"" << (opidx == OpHigh ? "high" : "low") << "\"\n";
        pinMode(dpins[pinidx].i, OUTPUT);
        digitalWrite(dpins[pinidx].i, opidx == OpHigh ? HIGH : LOW);
        return;
      case ArgNone:
        if (pinidx < ndpins) {
          pinMode(dpins[pinidx].i, INPUT);
          getStream()
            << "value of digital pin \"" << dpins[pinidx].n << "\": "
            << (digitalRead(dpins[pinidx].i) ? "high\n" : "low\n");
        } else {
          auto& p = apins[pinidx - ndpins];
          getStream()
            << "value of analog pin \"" << p.n <<"\": "
            << analogRead(p.i) << "\n";
        }
        return;
      default:
        break;
    }

    unsigned int val(0);
    switch (getUInt(val, 0, 255, 10 /* base */, true /* reparse */)) {
      case ArgOk:
        /* TODO: we don't check if the current pin supports PWM at all! */
        getStream()
          << "setting PWM of digital pin \"" << m_pinNames[pinidx] << "\" to \""
          << current() << "\"\n";
        pinMode(dpins[pinidx].i, OUTPUT);
        analogWrite(dpins[pinidx].i, val);
        break;
      /* shouldn't happen, catched above */
      case ArgNone:
        break;
      case ArgTooBig:
        getStream() << "\""<< current() << "\" too big. analogWrite range: 0 .. 255\n";
        break;
      default:
        getStream() << "invalid argument, supported are {h, l, 0 .. 255}\n";
        break;
    }
  }
  virtual void processStreamData()
  {
    CliBase::run();
  }
  virtual void reset()
  {
    if (not isConnected()) {
      return;
    }

    if (getClient()) {
      Serial << "telnet connection closed (" << getClient().remoteIP().toString() << ")\n";
    } else {
      Serial << "telnet connection closed (unknown IP)\n";
    }

    TelnetClient::reset();
  }
  void begin(const WiFiClient& client)
  {
    if (isConnected()) {
      return;
    }

    TelnetClient::begin(client);

    Serial << "telnet client connection (" << getClient().remoteIP().toString() << ")\n";

    /* make sure the new client must authenticate first */
    switchCommandSet(NotAuthenticated);

    getStream()
      << "Welcome to the Elektronik Workshop Telnet CLI!\n"
      << "Please enter the password for \"" << HostName << "\": ";
      ;
  }
private:
  const char* m_pinNames[npins];
};

const size_t NumClis = 1;

TelnetCli cli[NumClis];
TelnetServer server(cli, NumClis);

void setup()
{
  Serial.begin(115200);

  Serial << "connecting to " << WiFiSsid << " ...\n";
  WiFi.begin(WiFiSsid, WiFiPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial << "WiFi connected, IP: " << WiFi.localIP() << "\n";
  Serial << "setting up mDNS ... ";
  if (!MDNS.begin(HostName)) {
    Serial << "error setting up MDNS!\n";
  } else {
    MDNS.addService("telnet", "tcp", 23);
    Serial << "published host name: " << HostName << "\n";
  }
}

void loop()
{
  server.run();
}
#if 0
/* We use the connected callback to register our service at the mDNS-responder
 */
 void onWiFiConnected(const WiFiEventStationModeConnected& /*event*/)
 {
 }
auto wiFiConnectHandler = WiFi.onStationModeConnected(&onWiFiConnected);
#endif
