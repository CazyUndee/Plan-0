import socket, time, os

s = socket.socket()
s.settimeout(5)
s.connect(("127.0.0.1", 4444))
print("Connected to monitor")
time.sleep(5)

# Send screendump command
cmd = "screendump C:\\Users\\roone\\DESKTOP-QK3UG2M\\Downloads\\projects\\os\\bin\\screen.ppm\n"
s.send(cmd.encode())
time.sleep(1)

try:
    resp = s.recv(4096)
    print("Response:", resp.decode("ascii", errors="replace"))
except:
    pass
s.close()

# Check if screendump was created
scr_path = r"C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os\bin\screen.ppm"
if os.path.exists(scr_path):
    print(f"Screendump created: {os.path.getsize(scr_path)} bytes")
else:
    print("No screendump file created")
