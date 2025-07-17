# VPN Project

The project is divided into several stages: separate implementations of client and server, routing and network interface configuration. The server is deployed on a hosting service for testing. Client and server setups are provided for both Linux and Windows.

## Project structure

- `linux/` — source code and scripts for Linux:
  - `src/` — `server.cpp`, `client.cpp`, `tun.cpp`, `tun.h`
  - `scripts/` — `server_setup.sh`, `client_setup.sh`
- `windows/` — source code and scripts for Windows:
  - `client.cpp`, `tun.h`, `tun.cpp`, `wintun.h`, `client_setup.ps1`, `wintun.dll`

## Implementation stages

1. Creating TUN interfaces and reading/writing IP packets on Linux and Windows.
2. Implementing UDP server and client handling TUN traffic.
3. Configuring routing and NAT using shell scripts on Linux.
4. Separate Windows client implementation with PowerShell setup script.

## How to run (Linux)

1. Build server and client:
   ```bash
   g++ -o server linux/src/server.cpp linux/src/tun.cpp
   g++ -o client linux/src/client.cpp linux/src/tun.cpp
   ```
2. Run interface setup scripts (as root):
   ```bash
   sudo bash linux/scripts/server_setup.sh
   sudo bash linux/scripts/client_setup.sh
   ```
3. Run server and client:
   ```bash
   sudo ./server [listen_ip]
   sudo ./client <server_ip>
   ```

## How to run (Windows)

1. Build the client using appropriate compiler (Visual Studio, MinGW).
2. Run the PowerShell script to set up the TUN interface:
   ```powershell
   .\client_setup.ps1
   ```
3. Run the client.

---

## Notes

- Running programs with administrator/root privileges is required to work with TUN interfaces.
- The server runs on a Linux hosting with a public IP address.
- Windows uses `wintun.dll` for TUN functionality.

---

## License
MIT License

Copyright (c) 2025 NeytonBakker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
