#!/usr/bin/python3

import socket as sk

ip = "172.16.45.1"
port_range = "1-6000"
services = { 21:"ftp", 22:"ssh", 53:"dns", 80:"http", 135:"rpc", \
             139:"smb", 443:"https", 445:"smb", 514:"syslog", \
             3306:"mysql", 5985:"winrm" }


low_p = int( port_range.split("-")[0] )
high_p = int( port_range.split("-")[1] )

print("Scanning " + ip + " ports " + str(low_p) + "-" + str(high_p) )
for p in range(low_p, high_p):
    addr=(ip,p)
    s = sk.socket(sk.AF_INET, sk.SOCK_STREAM)
    s.settimeout(1000) 
    if not (s.connect_ex(addr)):
        if addr[1] in services.keys():      
            print(ip + ":" + str(p) + " - open (" + services.get(addr[1]) + ")")
    else:
        #print(ip + ":" + str(p) + " - closed")
        continue
s.close()
