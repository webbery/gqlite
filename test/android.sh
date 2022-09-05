#!/bin/bash

# /sdcard
adb root
adb shell avbctl disable-verification
adb reboot
adb wait-for-device
adb root
adb remount
adb shell mount -o rw,remount /

adb shell mkdir /sdcard/test
adb shell mkdir /sdcard/test/edge
adb shell mkdir /sdcard/test/error
adb shell mkdir /sdcard/test/graphs
adb shell mkdir /sdcard/test/query
adb shell mkdir /sdcard/test/util
adb shell mkdir /sdcard/test/vertex
adb shell mkdir /sdcard/test/walk
adb push build/test/vnet /sdcard/test/
adb push build/test/storage /sdcard/test/
adb push build/test/movielens /sdcard/test/
adb push build/test/regress /sdcard/test/
# adb push build/test/query /sdcard/test/
adb push build/test/grammar /sdcard/test/
adb push test/edge/grammar.gql /sdcard/test/edge/
adb push test/error/errorinfo.gql /sdcard/test/error/
adb push test/query/grammar.gql /sdcard/test/query/
adb push test/util/grammar.gql /sdcard/test/util/
adb push test/vertex/grammar.gql /sdcard/test/vertex/
adb push test/walk/grammar.gql /sdcard/test/walk/
adb shell sh /sdcard/test/vnet
adb shell sh /sdcard/test/storage
adb shell sh /sdcard/test/grammar
# adb shell sh /sdcard/test/query
adb shell sh /sdcard/test/movielens
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/vertex --outputdir=/sdcard/test/vertex
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/edge --outputdir=/sdcard/test/edge
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/error --outputdir=/sdcard/test/error
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/util --outputdir=/sdcard/test/util
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/walk --outputdir=/sdcard/test/walk
