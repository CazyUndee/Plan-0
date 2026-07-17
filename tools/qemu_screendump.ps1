param(
    [string]$IsoPath,
    [string]$OutputDir
)

$qemu = "C:\Users\roone.DESKTOP-QK3UG2M\qemu\qemu-system-x86_64w.exe"

# Start QEMU with monitor on TCP
$proc = Start-Process -FilePath $qemu -ArgumentList @(
    "-cdrom", "`"$IsoPath`"",
    "-m", "128",
    "-accel", "tcg",
    "-no-reboot",
    "-monitor", "tcp:127.0.0.1:4444,server,nowait"
) -WindowStyle Minimized -PassThru

Start-Sleep -Seconds 15

# Connect to QEMU monitor and send commands
try {
    $client = New-Object System.Net.Sockets.TcpClient
    $client.Connect('127.0.0.1', 4444)
    $stream = $client.GetStream()
    $writer = New-Object System.IO.StreamWriter($stream)
    $writer.AutoFlush = $true
    $reader = New-Object System.IO.StreamReader($stream)

    Start-Sleep -Milliseconds 1000
    
    # Send screendump command
    $screendumpPath = Join-Path $OutputDir "screendump.ppm"
    $writer.WriteLine("screendump $screendumpPath")
    Start-Sleep -Seconds 3
    
    # Read available output
    $output = ""
    while ($stream.DataAvailable) {
        $output += $reader.ReadLine() + "`n"
    }
    Write-Host "Monitor output:"
    Write-Host $output
    
    $writer.WriteLine("info registers")
    Start-Sleep -Milliseconds 500
    $output2 = ""
    while ($stream.DataAvailable) {
        $output2 += $reader.ReadLine() + "`n"
    }
    Write-Host "Registers:"
    Write-Host $output2
    
    $writer.WriteLine("quit")
    Start-Sleep -Seconds 2
    $client.Close()
} catch {
    Write-Host "Monitor error: $_"
}

Start-Sleep -Seconds 3
$proc.Kill()

if (Test-Path $screendumpPath) {
    Write-Host "Screendump saved: $( (Get-Item $screendumpPath).Length ) bytes"
} else {
    Write-Host "No screendump file created"
}
