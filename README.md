# Instructions
Use arch linux, install build-essential cmake cpprestsdk grpc boost

cpprestsdk is in aur

put keys into a keys dir, they will be copied into build dir

inside the grpc directory run ./doit.sh this will download ultimateq latest proto files and generate the include files we use

make a build folder cd into it cmake .. && make


# TODO
* Move all 3rd party libs into a vendor folder, maybe add cpprestsdk to that instead of using a system installed version
* Let IRC users save a default location query so they can just type .weather
* Add the sunrize/set info
* add a forecast lookup command
* store location into a long term cache, and weather results into a short term cache
