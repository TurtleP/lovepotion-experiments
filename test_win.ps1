if (Get-Command "telnet" -ErrorAction SilentlyContinue)
{
    $timestamp = [int][double]::Parse((Get-Date -UFormat %s))
    New-Item -ItemType Directory -Path "./logs"
    telnet 192.168.50.29 8000 -f "./logs/telnet_$timestamp.log"
}
else
{
    Write-Host "telnet is not installed, please install it by running 'pkgmgr /iu:`"TelnetClient`"' in Command Prompt."
    exit 1
}
