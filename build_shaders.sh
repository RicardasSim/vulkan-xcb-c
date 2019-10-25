#!/bin/bash

glslangValidator -V shaders/simple.vert -o shaders/simple.vert.spv
glslangValidator -V shaders/simple.frag -o shaders/simple.frag.spv
