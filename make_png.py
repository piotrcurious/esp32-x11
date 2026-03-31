import zlib
import struct
import sys
import os

def make_png(width, height, data):
    pixel_data = bytearray()
    for y in range(height):
        pixel_data.append(0) # Filter type
        for x in range(width):
            idx = (y * width + x) * 4
            if idx + 2 >= len(data):
                b, g, r = 0, 0, 0
            else:
                b = data[idx]
                g = data[idx+1]
                r = data[idx+2]
            pixel_data.extend([r, g, b])

    def make_chunk(type, content):
        return struct.pack(">I", len(content)) + type + content + struct.pack(">I", zlib.crc32(type + content) & 0xffffffff)

    png = b"\x89PNG\r\n\x1a\n"
    png += make_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
    png += make_chunk(b"IDAT", zlib.compress(pixel_data))
    png += make_chunk(b"IEND", b"")
    return png

def generate_dummy(width, height, output_file):
    # Create a dummy RGB data (red square)
    data = bytearray()
    for y in range(height):
        for x in range(width):
            data.extend([0, 0, 255, 255]) # B, G, R, A
    png = make_png(width, height, data)
    with open(output_file, "wb") as f:
        f.write(png)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 make_png.py <width> <height> <raw_data_file> <output_png>")
        print("   OR: python3 make_png.py dummy")
        sys.exit(1)

    if sys.argv[1] == "dummy":
        generate_dummy(200, 100, "screenshot.png")
        print("Dummy screenshot generated as screenshot.png")
        return

    width = int(sys.argv[1])
    height = int(sys.argv[2])
    raw_file = sys.argv[3]
    output_file = sys.argv[4]

    if not os.path.exists(raw_file):
        print(f"Error: {raw_file} not found")
        sys.exit(1)

    with open(raw_file, "rb") as f:
        data = f.read()

    png = make_png(width, height, data)
    with open(output_file, "wb") as f:
        f.write(png)

if __name__ == "__main__":
    main()
