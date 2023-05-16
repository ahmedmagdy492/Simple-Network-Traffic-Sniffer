#!/bin/bash


sudo ip tuntap add mode tap user $(whoami)
sudo ip link set tap0 up
sudo ip addr add 192.168.80.1/24 dev tap0
sudo sysctl net.ipv4.ip_forward=1
sudo iptables -A FORWARD -i eth0 -o tap0 -j ACCEPT
sudo iptables -A FORWARD -i tap0 -o eth0 -j ACCEPT
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
