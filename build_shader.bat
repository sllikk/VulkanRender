@echo off
echo HELLO WORLD

IF EXIST shaders echo SHADERS_FILES_FOUND
cd shaders

glslc.exe vertex_shader.vert -o vert.spv
glslc.exe fragment_shader.vert -o vert.spv

mkdir 

