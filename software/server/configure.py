#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# @description      Configure controlpad and upload new configuration to it
#
# @author           Andrea Benini
# @date             2025-04-20
# @license          GNU Affero General Public License v3.0
# @see              CLI utility for configuring controlpad profiles with an easy to use utility
#
# pyright: reportMissingImports=false
#
import sys
try:
    import time
    import yaml
    import json
    import serial
    import argparse
except Exception as E:
    print(f"\nError while loading libraries: {E}\n", file=sys.stderr)
    sys.exit(1)


def serialSend(jsonData, serialPort, timeout=5, retries=3):
    stream = ('#CONFIG#BEGIN#'+jsonData+'#CONFIG#END#\n').encode('utf-8')
    print(f"- Sending yaml configuration to   [{serialPort}]...")
    print(f"    {stream}")
    for attempt in range(retries):
        try:
            if not serialPort:
                raise ValueError(f"ERROR: Invalid serial device ({serialPort})")
            serialHandler = serial.Serial(
                port=serialPort,
                baudrate=115200,
                timeout=timeout,
                write_timeout=timeout
            )
            # Open and close quickly to reset the connection
            serialHandler.close()
            time.sleep(0.5)
            serialHandler.open()
            # Send the data in chunks of 64 bytes
            chunkSize = 64
            for i in range(0, len(stream), chunkSize):
                chunk = stream[i:i+chunkSize]
                serialHandler.write(chunk)                # Encode the string to bytes and add a newline at the end
                serialHandler.flush()
                time.sleep(0.1)
            print(f"- Operation completed successfully on {serialPort}\n")
            return

        except serial.SerialTimeoutException:
            print(f"  Write timeout on attempt {attempt+1}, retrying...")
            time.sleep(2)
        except serial.SerialException as E:
            print(f"ERROR: Serial port issue: {str(E)}")
            if attempt < retries - 1:
                print("Retrying...")
                time.sleep(2)
            else:
                raise ValueError(f"ERROR: Cannot open serial port {serialPort} or send data: {str(E)}")
        except Exception as E:
            raise ValueError(str(E))
        finally:
            try:
                serialHandler.close()
                time.sleep(0.5)
            except Exception:
                pass
    raise ValueError(f"ERROR: Failed to send data after {retries} attempts")


def loadYAML(fileYAML):
    Content = None
    try:
        if not fileYAML:
            raise ValueError("yaml file cannot be an empty string")
        print(f"- Loading yaml configuration file [{fileYAML}]")
        # Open and read the YAML file
        with open(fileYAML, 'r') as yamlHandler:
            Content = yaml.safe_load(yamlHandler)  # Use safe_load to prevent arbitrary code execution
            # Convert the YAML content to a JSON string with ensure_ascii set to false.
            Content = json.dumps(Content, ensure_ascii=False)
        return Content
    except Exception as E:
        print(f"ERROR: {str(E)}")
        raise ValueError("Failed to load the YAML file")


def main():
    # Parse input parameters
    parser = argparse.ArgumentParser(description="Load a YAML configuration file and transfer it to controlpad over serial port.")
    parser.add_argument("-f", "--file", required=True, help="YAML configuration file for controlpad")
    parser.add_argument("-p", "--port", required=True, help="Serial port device name (e.g. /dev/ttyUSB0|ttyACM0|...)")
    args = parser.parse_args()
    try:
        jsonData = loadYAML(args.file)          # Convert YAML to JSON
        if jsonData:
            serialSend(jsonData, args.port)
        else:
            print("No yaml configuration data to send, aborting transfer")
        sys.exit(0)
    except (KeyboardInterrupt, FileNotFoundError, yaml.YAMLError, TypeError, ValueError) as e:
        print(f"\n{str(e)}\nERROR: Aborting script\n")
    except Exception as e:
        print(f"\nERROR: {str(e)}\n")
    sys.exit(1)

if __name__ == "__main__":
    main()
