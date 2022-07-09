import serial
from time import sleep
import sys
import argparse
import re
import csv
# Parse arguments
parser = argparse.ArgumentParser(description="Controlling two CC1352P's to send packets back and forth, and calculate RSSI")
parser.add_argument("--rxcomport", dest="rxComPort", help="COM port for RX CC1352P launchpad. ")
parser.add_argument("--txcomport", dest="txComPort", help="COM port for TX CC1352P launchpad. ")
parser.add_argument("--csvfile", dest="csvfile", help="CSV file that the results are written to. ")
args = parser.parse_args()
rxComPort = args.rxComPort
txComPort = args.txComPort
csvfile = args.csvfile
print("csvfile = ", csvfile)
print("TX com port = ", txComPort)
print("RX com port = ", rxComPort)

# Open COM ports:
serTx = serial.Serial("COM13", 115200)
serRx = serial.Serial("COM16", 115200)

# Clear the RX buffer
serTx.flushInput()
serTx.flushOutput()
serRx.flushInput()
serRx.flushOutput()

# Sleep for 100 ms:
sleep(0.1)
freqs = []
rssi = []
# Go through all frequencies in 2 MHz steps:
freq = 2400
while freq <= 2480:
	# start RX:
	serRx.write(bytes("RX "+str(freq)+"\r", "utf-8"))
	# Sleep for 0.1 secs
	sleep(0.1)
	# start TX transmission:
	serTx.write(bytes("TX "+str(freq)+"\r", "utf-8"))
	sleep(0.2)
	resultString = serRx.readline().decode("utf-8")
	# Check if result is OK:
	if "ERROR" in resultString:
		print(resultString)
	else:
		freqs.append(freq)
		rssi.append(int(re.search(r"-?\d+", resultString).group()))
		if csvfile == None:
			print("freq = ", freq)
			print(resultString)
		# print("RSSI = ", rssi)
		freq += 2

# Close COM ports:
serTx.close()
serRx.close()
print(freqs)
print(rssi)
# Save results to CSV file if applicable
if csvfile != None:
	with open(csvfile, "w+", newline='') as file:
		write = csv.writer(file)
		write.writerow(freqs)
		write.writerow(rssi)
	print("Done! results saved to file")