$adapterName = "MyVPNAdapter"
$vpnServerIP = "37.233.82.11"
$vpnIPAddress = "10.8.0.2"
$vpnPrefixLength = 24

while (-not (Get-NetAdapter -Name $adapterName -ErrorAction SilentlyContinue)) {
    Start-Sleep -Seconds 1
}


New-NetIPAddress -InterfaceAlias $adapterName -IPAddress $vpnIPAddress -PrefixLength $vpnPrefixLength -SkipAsSource $false -ErrorAction Stop | Out-Null

$mainRoute = Get-NetRoute -DestinationPrefix "0.0.0.0/0" -AddressFamily IPv4 | Sort-Object RouteMetric | Select-Object -First 1
$mainIfIndex = $mainRoute.ifIndex
$mainGateway = $mainRoute.NextHop

if (-not (Get-NetRoute -DestinationPrefix "$vpnServerIP/32" -ErrorAction SilentlyContinue)) {
    New-NetRoute -DestinationPrefix "$vpnServerIP/32" -InterfaceIndex $mainIfIndex -NextHop $mainGateway -RouteMetric 1 | Out-Null
}


$vpnGateway = ($vpnIPAddress.Split('.')[0..2] + '1') -join '.'
$vpnIfIndex = (Get-NetAdapter -Name $adapterName).ifIndex


Get-NetRoute -DestinationPrefix "0.0.0.0/0" -InterfaceIndex $vpnIfIndex -ErrorAction SilentlyContinue | Remove-NetRoute -Confirm:$false -ErrorAction SilentlyContinue

New-NetRoute -DestinationPrefix "0.0.0.0/0" -InterfaceIndex $vpnIfIndex -NextHop $vpnGateway -RouteMetric 1 | Out-Null

Write-Output " VPN IP: $vpnIPAddress/$vpnPrefixLength"
Write-Output " Default route via: $vpnGateway (ifIndex=$vpnIfIndex)"
Write-Output " VPN Adapter: $adapterName"

