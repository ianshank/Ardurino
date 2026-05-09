# Reset the XIAO and capture serial output
param(
  [string]$Port = "COM12",
  [int]$Seconds = 20
)

$esptool = "C:\Users\iansh\.platformio\packages\tool-esptoolpy\esptool.py"
$python = "C:\Users\iansh\.platformio\penv\Scripts\python.exe"

# Hard reset (no-stub mode to avoid stub upload failure on this board)
& $python $esptool --chip esp32s3 --port $Port --no-stub --after hard_reset run 2>&1 | Select-Object -Last 3
Start-Sleep -Milliseconds 800

try {
  $sp = New-Object System.IO.Ports.SerialPort $Port,115200,None,8,One
  $sp.ReadTimeout = 500
  $sp.Open()
  $sp.DtrEnable = $true
  Write-Host "[host] port opened, listening..."
  $sb = New-Object System.Text.StringBuilder
  $deadline = (Get-Date).AddSeconds($Seconds)
  while ((Get-Date) -lt $deadline) {
    try {
      $line = $sp.ReadLine()
      $null = $sb.AppendLine($line)
      Write-Host $line
    } catch [System.TimeoutException] { }
  }
  $sp.Close()
  $sb.ToString() | Out-File C:\Users\iansh\OneDrive\Documents\Ardurino\tmp_serial.txt -Encoding UTF8
  Write-Host "---Captured $($sb.Length) chars---"
} catch {
  Write-Host "ERR: $($_.Exception.Message)"
}
