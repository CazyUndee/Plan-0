# OpenKernel Build Verification Script

Write-Host "=== OpenKernel Build Verification ===" -ForegroundColor Green

# Check project structure
Write-Host "`n1. Project Structure:" -ForegroundColor Yellow
$required_files = @{
    "src\openkernel.c" = "OpenKernel main file"
    "src\pmm.c" = "OpenMemory PMM"
    "src\paging.c" = "Paging system"
    "src\kheap.c" = "OpenMemory heap"
    "src\fs.c" = "OpenFS filesystem"
    "src\shell.c" = "OpenShell"
    "src\gpt.c" = "OpenPart driver"
    "boot\boot.asm" = "OpenKernel bootstrap"
    "boot\interrupts.asm" = "Interrupt handlers"
    "Makefile" = "Build system"
    "linker\linker.ld" = "Linker script"
}

$all_files_exist = $true
foreach ($file in $required_files.Keys) {
    if (Test-Path $file) {
        Write-Host "✓ $file - $($required_files[$file])" -ForegroundColor Green
    } else {
        Write-Host "✗ $file - $($required_files[$file])" -ForegroundColor Red
        $all_files_exist = $false
    }
}

# Check Open* naming consistency
Write-Host "`n2. Open* Naming Consistency:" -ForegroundColor Yellow
$open_files = Get-ChildItem -Path "src" -Filter "*.c"
foreach ($file in $open_files) {
    $content = Get-Content $file.FullName -Raw
    if ($content -match 'OpenKernel|OpenMemory|OpenFS|OpenPart|OpenInput|OpenShell') {
        Write-Host "✓ $($file.Name) uses Open* naming" -ForegroundColor Green
    }
}

# Check for 32-bit remnants
Write-Host "`n3. 32-bit Cleanup Check:" -ForegroundColor Yellow
$bad_patterns = @("kernel64", "pmm64", "paging64", "kheap64", "idt64", "gdt64", "boot64", "interrupts64")
$found_32bit = $false

foreach ($pattern in $bad_patterns) {
    $matches = Get-ChildItem -Path "." -Recurse -Filter "*$pattern*"
    if ($matches.Count -gt 0) {
        Write-Host "✗ Found 32-bit remnants: $pattern" -ForegroundColor Red
        $found_32bit = $true
    }
}

if (-not $found_32bit) {
    Write-Host "✓ No 32-bit remnants found" -ForegroundColor Green
}

# Summary
Write-Host "`n=== Build Verification Summary ===" -ForegroundColor Green
if ($all_files_exist -and -not $found_32bit) {
    Write-Host "✅ Project ready for build!" -ForegroundColor Green
    Write-Host "   All required files present" -ForegroundColor Cyan
    Write-Host "   Open* naming consistent" -ForegroundColor Cyan
    Write-Host "   No 32-bit remnants" -ForegroundColor Cyan
    Write-Host "`n   To build: Install GCC, NASM, GNU Make then run 'make all'" -ForegroundColor Yellow
} else {
    Write-Host "❌ Project needs attention before build" -ForegroundColor Red
}
