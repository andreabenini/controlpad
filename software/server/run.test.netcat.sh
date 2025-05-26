#!/usr/bin/env bash
echo ""
echo "NETCAT 9100, server side for testing controlpad"
echo "------------------------------------ Ctrl+C to abort ---"
# Server side listening, output
# --verbose
# --udp|--tcp
# stdbuf -o0 nc --local-port=9100 --listen --verbose
stdbuf -o0 nc -l -p 9100 --verbose

# Client side
# nc localhost 9100
# telnet localhost 9100
#    Ctrl+]
#       mode character
#       set crlf
# expect -c '
#   spawn telnet YOUR_SERVER_IP YOUR_PORT
#   expect "Escape character is"
#   send "\x1d" 
#   expect "telnet>"
#   send "mode character\r"
#   send "\r"
#   interact
# '
