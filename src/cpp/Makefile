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

$(BIN):
	mkdir -p $(BIN)

$(BIN)Brain.o: $(BIN) $(ROBOTINO)Brain.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_Bumper.o: $(BIN) $(ROBOTINO)_Bumper.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_CompactBha.o: $(BIN) $(ROBOTINO)_CompactBha.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_Odometry.o: $(BIN) $(ROBOTINO)_Odometry.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_OmniDrive.o: $(BIN) $(ROBOTINO)_OmniDrive.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)_DistanceSensors.o: $(BIN) $(ROBOTINO)_DistanceSensors.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Vector.o: $(BIN) $(GEOMETRY)Vector.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Coordinate.o: $(BIN) $(GEOMETRY)Coordinate.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Angle.o: $(BIN) $(GEOMETRY)Angle.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)AngularCoordinate.o: $(BIN) $(GEOMETRY)AngularCoordinate.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)Scalar.o: $(BIN) $(GEOMETRY)Scalar.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)VolumeCoordinate.o: $(BIN) $(GEOMETRY)VolumeCoordinate.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)TcpSocket.o: $(BIN) $(TCP)TcpSocket.cpp
	$(CC) $(CFLAGS) -c -o $@ $?

$(BIN)KinectReader.o: $(BIN) $(KINECT)KinectReader.cpp
	$(CC) $(CFLAGS) -c -o $@ $?


test: test.cpp $(BIN)Vector.o $(BIN)Coordinate.o $(BIN)Angle.o $(BIN)Scalar.o
	$(CC) $(CFLAGS) -o $@ $?

#$(AUX)options: $(AUX)options.cpp
#	g++ -o $@ $? -lboost_program_options

remake: clean main

clean: $(BIN)
	-rm main
	-rm test
#	-rm $(AUX)options
	-rm $(BIN)*.o
