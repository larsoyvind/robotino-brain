# Makefile for the RobotinoXT/Brain-project

CC=g++
CFLAGS=-Wall -Werror -g -O -std=c++0x -pthread

API2LIB=rec_robotino_api2
BIN=bin/
ROBOTINO=robotino/
KINECT=kinect/
TCP=tcp/
AUX=aux/
GEOMETRY=geometry/

main: main.cpp Control.cpp $(BIN)Brain.o $(BIN)_Bumper.o $(BIN)_CompactBha.o $(BIN)_Odometry.o $(BIN)_OmniDrive.o $(BIN)_DistanceSensors.o $(BIN)Vector.o $(BIN)Coordinate.o $(BIN)Angle.o $(BIN)AngularCoordinate.o $(BIN)Scalar.o $(BIN)VolumeCoordinate.o $(BIN)TcpSocket.o $(BIN)KinectReader.o
	$(CC) $(CFLAGS) -l $(API2LIB) -o $@ main.cpp Control.cpp $(BIN)Brain.o $(BIN)_Bumper.o $(BIN)_CompactBha.o $(BIN)_Odometry.o $(BIN)_OmniDrive.o $(BIN)_DistanceSensors.o $(BIN)Vector.o $(BIN)Coordinate.o $(BIN)Angle.o $(BIN)AngularCoordinate.o $(BIN)Scalar.o $(BIN)VolumeCoordinate.o $(BIN)TcpSocket.o $(BIN)KinectReader.o


$(BIN)Brain.o: $(ROBOTINO)Brain.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_Bumper.o: $(ROBOTINO)_Bumper.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_CompactBha.o: $(ROBOTINO)_CompactBha.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_Odometry.o: $(ROBOTINO)_Odometry.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_OmniDrive.o: $(ROBOTINO)_OmniDrive.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_DistanceSensors.o: $(ROBOTINO)_DistanceSensors.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Vector.o: $(GEOMETRY)Vector.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Coordinate.o: $(GEOMETRY)Coordinate.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Angle.o: $(GEOMETRY)Angle.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)AngularCoordinate.o: $(GEOMETRY)AngularCoordinate.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Scalar.o: $(GEOMETRY)Scalar.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)VolumeCoordinate.o: $(GEOMETRY)VolumeCoordinate.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)TcpSocket.o: $(TCP)TcpSocket.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)KinectReader.o: $(KINECT)KinectReader.cpp
	$(CC) $(CFLAGS) -c -o $@ $?


test: test.cpp $(BIN)Vector.o $(BIN)Coordinate.o $(BIN)Angle.o $(BIN)Scalar.o
	$(CC) $(CFLAGS) -o $@ $?

$(AUX)options: $(AUX)options.cpp
	g++ -o $@ $? -lboost_program_options

remake: clean main

clean:
	-rm main
	-rm test
	-rm $(AUX)options
	-rm $(BIN)*.o
