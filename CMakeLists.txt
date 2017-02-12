cmake_minimum_required(VERSION 3.7)
project(Toco VERSION 0.1 LANGUAGES CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM CONFIG REQUIRED)
