#!/bin/bash

cd Bezier
g++ -Wall -o Assignment Assignment.cpp -lGL -lGLU -lglut -lGLEW
cd ..

cd Terrain
g++ -Wall -o Terrain Terrain.cpp -lGL -lGLU -lglut -lGLEW
cd ..
