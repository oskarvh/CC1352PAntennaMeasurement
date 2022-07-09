# CC1352P Antenna Measurement
This project uses two CC1352P-2 launchpad boards to measure RSSI for different antennas on the 2.4 GHz band. 
Use Code Composer Studio V11.2 and the SimpleLink CC13xx SDK to program the CC1352P boards. 
Use Python 3.7 for the data aquisition script, which controls the launchpads via UART. 

To import the CC1352P project into CCS: Open a new workspace -> project->import, and select the CC1352P_antenna_measurement directory. 

To use the python script: python measure_antenna_parameters.py --rxcomport <COMPORT_FOR_RX_DEVICE> --txcomport <COMPORT_FOR_RX_DEVICE> --csvfile <CSV_FILE.csv>