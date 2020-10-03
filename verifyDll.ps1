# **********************************************************************
#  PoC: verifyDll.ps1	
#  Author: lorenzoinvidia
#
#  Description:
#  Check for signed dlls in wow64 using signtool.exe
#
# ********************************************************************** 

$currwd = $(Get-Location);
$syswow = "C:\Windows\SysWOW64\";
$sdk = "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\";

$list = (Get-childitem $syswow);

Set-Location $sdk;

foreach ($l in $list){ 
    if ($l.Name -match "\.dll"){
        $opt = $syswow + $l.name;

        if ( $(.\signtool.exe verify $opt 2>&1 $null) -match "^Successfully" ){
            Write-Output "$($l.Name) signed!"; 
        }
    }
}

Set-Location $currwd;