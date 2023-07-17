import io

def main():
    fileName = "E2.txt"
    outfileName = "E2.h"

    with open(fileName, 'r') as f:
        data = f.read();

        data_array = data.split(" ")
        # print(data_array)

        with open(outfileName, 'w') as outfile:
            array_len = len(data_array)
            outfile.write(f"unsigned char rawData[{array_len}] = ")
            outfile.write("{\n\t")

            for i in range(array_len-1):
                element = data_array[i].replace("\n", "")
                outfile.write(f"0x{element}, ")

            
            element = data_array[array_len-1].replace("\n", "")
            outfile.write(f"0x{element}")

            outfile.write("\n};\n")

if __name__ == "__main__":
    main()
