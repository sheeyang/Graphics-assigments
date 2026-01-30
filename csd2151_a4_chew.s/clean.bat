rmdir /Q /S .vs
rmdir /Q /S x64
rmdir /Q /S bin

del /A:H *.zip

cd Shaders
call clean.bat
cd ..
