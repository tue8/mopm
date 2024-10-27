# mopm
A Windows package manager.

## Installation
Run these commands from **PowerShell** as **Administrator**
```powershell
Set-ExecutionPolicy RemoteSigned
iex ((New-Object System.Net.WebClient).DownloadString('https://server.cyberpho.be/mopm.ps1'))
```

### Libraries:
- Christophe Devine's FIPS-180-2 compliant SHA-256 implementation
- libcurl
- jansson
- libarchive (+ zlib, bzip)