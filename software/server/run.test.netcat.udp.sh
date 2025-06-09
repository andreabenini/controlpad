#!/usr/bin/env bash
echo ""
echo "NETCAT 3000 [UDP], server side for testing controlpad"
echo "------------------------------------ Ctrl+C to abort ---"
# Server side listening, output
# --verbose
# --udp|--tcp
# stdbuf -o0 nc --local-port=9100 --listen --verbose
stdbuf -o0 nc --udp -l -p 3000 --verbose

