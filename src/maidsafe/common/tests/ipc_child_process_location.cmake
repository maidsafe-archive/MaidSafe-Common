file(TO_NATIVE_PATH "${IpcChildProcessLocation}" IpcChildProcessLocation)
string(REPLACE "\\" "\\\\" IpcChildProcessLocation "${IpcChildProcessLocation}")
configure_file(${InputFile} ${OutputFile} @ONLY)
