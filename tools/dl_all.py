import urllib.request, os, lzma, tarfile, io

url = "http://ftp.debian.org/debian/pool/main/g/grub2/grub-pc-bin_2.14-2_amd64.deb"
osdir = r"C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os"

req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
data = urllib.request.urlopen(req, timeout=120).read()
print("Downloaded " + str(len(data)) + " bytes")

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
        print("Decompressed to " + str(len(xz_data)) + " bytes")
        with tarfile.open(fileobj=io.BytesIO(xz_data), mode="r") as tar:
            count = 0
            for m in tar.getmembers():
                if m.isfile():
                    tar.extract(m, path=osdir)
                    count += 1
            print("Extracted " + str(count) + " files")
    
    pos = content_start + size
    if pos % 2 != 0:
        pos += 1

# Verify
grub_dir = os.path.join(osdir, ".", "usr", "lib", "grub", "i386-pc")
mods = [f for f in os.listdir(grub_dir) if f.endswith(".mod")]
print(str(len(mods)) + " .mod files in " + grub_dir)
print("Done!")
