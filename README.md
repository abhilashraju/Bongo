# http-client-based-on-beast
An http client base on beast/ boost asio
use the following command to build . Replace the path to boost library based on your installation
clear ;g++ -std=c++17 main.cpp -I path_to_boost_library -pthread -lssl -lcrypto -o beastex -g 
