import urllib.request, os, lzma, tarfile, io

url = "http://ftp.debian.org/debian/pool/main/g/grub2/grub-pc-bin_2.14-2_amd64.deb"
osdir = r"C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os"

req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
data = urllib.request.urlopen(req, timeout=120).read()

pos = 0
if data[0:8] != b"!<arch>\n":
    print("Not an ar archive")
    exit(1)

pos = 8
while pos < len(data):
    name = data[pos:pos+16].rstrip(b" ").decode("ascii", errors="replace")
    size_str = data[pos+48:pos+58].rstrip(b" ").decode("ascii", errors="replace")
    try:
        size = int(size_str)
    except:
        break
    content_start = pos + 60
    content = data[content_start:content_start+size]
    
    if "data.tar" in name:
        xz_data = lzma.decompress(content)
        with tarfile.open(fileobj=io.BytesIO(xz_data), mode="r") as tar:
            for m in tar.getmembers():
                if "boot_hybrid" in m.name or "cdboot" in m.name or "core.img" in m.name or "boot.img" in m.name or "diskboot" in m.name:
                    tar.extract(m, path=osdir)
                    extracted = os.path.join(osdir, m.name)
                    if os.path.exists(extracted):
                        print(f"Extracted: {m.name} ({m.size} bytes)")
    
    pos = content_start + size
    if pos % 2 != 0:
        pos += 1

print("Done!")
