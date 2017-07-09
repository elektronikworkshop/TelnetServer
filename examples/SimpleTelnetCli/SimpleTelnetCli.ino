/** SimpleTelnetCli, a simple TelnetServer example
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

template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

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
    switchCommandSet(Authenticated);
    setDefaultHandler(&TelnetCli::def);
    addCommand("help", &TelnetCli::help);
    addCommand("quit", &TelnetCli::quit);
    addCommand("led",  &TelnetCli::led);

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
      "help          -- print this help\n"
      "quit          -- terminate telnet session\n"
      "led {on, off} -- turn the built in LED on or off\n"
      ;
  }
  void quit()
  {
    reset();
  }
  void led()
  {
    size_t offon(0);
    switch (getOpt(offon, "off", "on")) {
      case ArgOk:
        digitalWrite(LED_BUILTIN, offon);
        return;
      case ArgNone:
        getStream() << "no argument, ";
        break;
      default:
        getStream() << "invalid argument \"" << current() << "\", ";
        return;
    }
    getStream() << "valid arguments: \"on\" or \"off\"\n";
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
  virtual void processStreamData()
  {
    CliBase::run();
  }
};

TelnetCli cli[MaxTelnetClients];
TelnetServer server(cli, MaxTelnetClients);

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
