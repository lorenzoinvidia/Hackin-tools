# Param(
#     [Parameter(Mandatory=$True)]
#     [String] $ip
# )

$ip = '172.16.146.1';
$ports = (22, 80, 443, 8080);

foreach ($p in $ports){
    try{
        $sock = New-Object System.Net.Sockets.TcpClient($ip,$p);
    }catch{};

    if ($sock -eq $null){
        echo $ip':'$p' - closed';  
    } else {
        echo $ip':'$p' - open';
    }
    $sock = $null;
}