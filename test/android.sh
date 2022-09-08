#!/bin/bash

adb push build/test/vnet /sdcard/vnet
adb shell sh /sdcard/vnet
adb push build/test/storage /sdcard/test/storage
adb push build/test/movielens /sdcard/movielens
adb push build/test/regress /sdcard/regress
adb push build/test/query /sdcard/query
adb push build/test/grammar /sdcard/grammar
