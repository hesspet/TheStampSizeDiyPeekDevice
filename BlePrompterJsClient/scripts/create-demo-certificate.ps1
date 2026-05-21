param(
    [Parameter(Mandatory = $true)]
    [string] $OutputDirectory,

    [string] $Passphrase = "BlePrompterDemo",

    [int] $RootCertificateYears = 5,

    [int] $ServerCertificateDays = 825
)

$ErrorActionPreference = "Stop"

function Get-LocalIpv4AddressList {
    $addresses = [System.Net.Dns]::GetHostAddresses([System.Net.Dns]::GetHostName()) |
        Where-Object {
            $_.AddressFamily -eq [System.Net.Sockets.AddressFamily]::InterNetwork `
                -and $_.ToString() -ne "127.0.0.1" `
                -and -not $_.ToString().StartsWith("169.254.")
        } |
        ForEach-Object { $_.ToString() } |
        Sort-Object -Unique

    return @("127.0.0.1") + @($addresses)
}

function New-SerialNumber {
    $serialNumber = New-Object byte[] 16
    $randomNumberGenerator = [System.Security.Cryptography.RandomNumberGenerator]::Create()
    $randomNumberGenerator.GetBytes($serialNumber)
    $randomNumberGenerator.Dispose()
    return $serialNumber
}

function New-RootCertificate {
    param(
        [string] $PfxPath,
        [string] $CerPath,
        [string] $Password
    )

    $rootKey = [System.Security.Cryptography.RSA]::Create(4096)
    $distinguishedName = [System.Security.Cryptography.X509Certificates.X500DistinguishedName]::new(
        "CN=BlePrompter lokale Demo Root CA")
    $request = [System.Security.Cryptography.X509Certificates.CertificateRequest]::new(
        $distinguishedName,
        $rootKey,
        [System.Security.Cryptography.HashAlgorithmName]::SHA256,
        [System.Security.Cryptography.RSASignaturePadding]::Pkcs1)

    $request.CertificateExtensions.Add(
        [System.Security.Cryptography.X509Certificates.X509BasicConstraintsExtension]::new($true, $false, 0, $true))
    $request.CertificateExtensions.Add(
        [System.Security.Cryptography.X509Certificates.X509KeyUsageExtension]::new(
            [System.Security.Cryptography.X509Certificates.X509KeyUsageFlags]::KeyCertSign -bor
            [System.Security.Cryptography.X509Certificates.X509KeyUsageFlags]::CrlSign -bor
            [System.Security.Cryptography.X509Certificates.X509KeyUsageFlags]::DigitalSignature,
            $true))
    $request.CertificateExtensions.Add(
        [System.Security.Cryptography.X509Certificates.X509SubjectKeyIdentifierExtension]::new(
            $request.PublicKey,
            $false))

    $notBefore = [System.DateTimeOffset]::Now.AddDays(-1)
    $notAfter = $notBefore.AddYears($RootCertificateYears)
    $certificate = $request.CreateSelfSigned($notBefore, $notAfter)

    [System.IO.File]::WriteAllBytes(
        $PfxPath,
        $certificate.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Pfx, $Password))
    [System.IO.File]::WriteAllBytes(
        $CerPath,
        $certificate.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Cert))
}

function New-ServerCertificate {
    param(
        [string] $RootPfxPath,
        [string] $ServerPfxPath,
        [string] $MetadataPath,
        [string] $Password,
        [string[]] $IpAddresses
    )

    $keyStorageFlags =
        [System.Security.Cryptography.X509Certificates.X509KeyStorageFlags]::Exportable -bor
        [System.Security.Cryptography.X509Certificates.X509KeyStorageFlags]::EphemeralKeySet
    $rootCertificate = [System.Security.Cryptography.X509Certificates.X509Certificate2]::new(
        $RootPfxPath,
        $Password,
        $keyStorageFlags)

    $serverKey = [System.Security.Cryptography.RSA]::Create(2048)
    $distinguishedName = [System.Security.Cryptography.X509Certificates.X500DistinguishedName]::new(
        "CN=BlePrompter lokale HTTPS-Demo")
    $request = [System.Security.Cryptography.X509Certificates.CertificateRequest]::new(
        $distinguishedName,
        $serverKey,
        [System.Security.Cryptography.HashAlgorithmName]::SHA256,
        [System.Security.Cryptography.RSASignaturePadding]::Pkcs1)

    $subjectAlternativeNames = [System.Security.Cryptography.X509Certificates.SubjectAlternativeNameBuilder]::new()
    $subjectAlternativeNames.AddDnsName("localhost")
    $subjectAlternativeNames.AddDnsName([System.Net.Dns]::GetHostName())

    foreach ($ipAddressText in $IpAddresses) {
        $subjectAlternativeNames.AddIpAddress([System.Net.IPAddress]::Parse($ipAddressText))
    }

    $request.CertificateExtensions.Add($subjectAlternativeNames.Build())
    $request.CertificateExtensions.Add(
        [System.Security.Cryptography.X509Certificates.X509BasicConstraintsExtension]::new($false, $false, 0, $true))
    $request.CertificateExtensions.Add(
        [System.Security.Cryptography.X509Certificates.X509KeyUsageExtension]::new(
            [System.Security.Cryptography.X509Certificates.X509KeyUsageFlags]::DigitalSignature -bor
            [System.Security.Cryptography.X509Certificates.X509KeyUsageFlags]::KeyEncipherment,
            $true))

    $enhancedKeyUsage = [System.Security.Cryptography.OidCollection]::new()
    $enhancedKeyUsage.Add([System.Security.Cryptography.Oid]::new("1.3.6.1.5.5.7.3.1")) | Out-Null
    $request.CertificateExtensions.Add(
        [System.Security.Cryptography.X509Certificates.X509EnhancedKeyUsageExtension]::new($enhancedKeyUsage, $true))

    $notBefore = [System.DateTimeOffset]::Now.AddDays(-1)
    $notAfter = $notBefore.AddDays($ServerCertificateDays)
    $serverCertificateWithoutKey = $request.Create(
        $rootCertificate,
        $notBefore,
        $notAfter,
        (New-SerialNumber))
    $serverCertificate = [System.Security.Cryptography.X509Certificates.RSACertificateExtensions]::CopyWithPrivateKey(
        $serverCertificateWithoutKey,
        $serverKey)

    [System.IO.File]::WriteAllBytes(
        $ServerPfxPath,
        $serverCertificate.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Pfx, $Password))

    $metadata = [ordered] @{
        createdAt = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ssK")
        hostName = [System.Net.Dns]::GetHostName()
        ipAddresses = $IpAddresses
    }
    $metadata | ConvertTo-Json | Set-Content -Encoding UTF8 -Path $MetadataPath
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null

$rootPfxPath = Join-Path $OutputDirectory "ble-prompter-demo-root-ca.pfx"
$rootCerPath = Join-Path $OutputDirectory "ble-prompter-demo-root-ca.cer"
$serverPfxPath = Join-Path $OutputDirectory "ble-prompter-demo-server.pfx"
$metadataPath = Join-Path $OutputDirectory "ble-prompter-demo-server.json"
$ipAddresses = @(Get-LocalIpv4AddressList)

if (-not (Test-Path $rootPfxPath) -or -not (Test-Path $rootCerPath)) {
    New-RootCertificate -PfxPath $rootPfxPath -CerPath $rootCerPath -Password $Passphrase
}

$shouldCreateServerCertificate = $true
if ((Test-Path $serverPfxPath) -and (Test-Path $metadataPath)) {
    $metadata = Get-Content -Raw -Path $metadataPath | ConvertFrom-Json
    $knownAddresses = @($metadata.ipAddresses) | Sort-Object -Unique
    $currentAddresses = @($ipAddresses) | Sort-Object -Unique
    $shouldCreateServerCertificate = (Compare-Object $knownAddresses $currentAddresses) -ne $null
}

if ($shouldCreateServerCertificate) {
    New-ServerCertificate `
        -RootPfxPath $rootPfxPath `
        -ServerPfxPath $serverPfxPath `
        -MetadataPath $metadataPath `
        -Password $Passphrase `
        -IpAddresses $ipAddresses
}

Write-Output "Root-Zertifikat: $rootCerPath"
Write-Output "Server-Zertifikat: $serverPfxPath"
Write-Output "Zertifizierte IP-Adressen:"
foreach ($ipAddressText in $ipAddresses) {
    Write-Output "  $ipAddressText"
}
