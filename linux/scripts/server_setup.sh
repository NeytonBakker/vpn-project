
set -e


ip tuntap add dev tun0 mode tun
ip addr add 10.8.0.1/24 dev tun0
ip link set dev tun0 up

sysctl -w net.ipv4.ip_forward=1


iptables -t nat -F
iptables -t nat -A POSTROUTING -s 10.8.0.0/24 -o eth0 -j MASQUERADE

echo "[+] Server TUN and NAT configured"