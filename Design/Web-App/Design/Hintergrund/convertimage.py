#!/usr/bin env python3

output = ""

count = 0

f = open(".background.jpg", "rb")

try:
    while (True):

        currentByte = f.read(1)

        if (currentByte == ""):
            break
        else:

            formattedChar = '0x' + format(ord(currentByte), '02X')

            if ( count == 20 ):
                output += formattedChar + ",\n"
                count = 0
            else:
                output += formattedChar + ", "
                count = count + 1

finally:
    f.close()

print(output)
