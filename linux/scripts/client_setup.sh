
set -e


ip tuntap add dev tun1 mode tun
ip addr add 10.8.0.2/24 dev tun1
ip link set dev tun1 up



echo "[+] Client TUN configured" 