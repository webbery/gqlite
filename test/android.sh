#!/bin/bash

adb root
adb shell avbctl disable-verification
adb reboot
adb root
adb remount
adb shell mount -o rw,remount /

adb shell mkdir /storage/emulated/0/test
adb shell mkdir /storage/emulated/0/test/edge
adb shell mkdir /storage/emulated/0/test/error
adb shell mkdir /storage/emulated/0/test/graphs
adb shell mkdir /storage/emulated/0/test/query
adb shell mkdir /storage/emulated/0/test/util
adb shell mkdir /storage/emulated/0/test/vertex
adb shell mkdir /storage/emulated/0/test/walk
adb push build/test/vnet /storage/emulated/0/test/
adb push build/test/storage /storage/emulated/0/test/
adb push build/test/movielens /storage/emulated/0/test/
adb push build/test/regress /storage/emulated/0/test/
# adb push build/test/query /storage/emulated/0/test/
adb push build/test/grammar /storage/emulated/0/test/
adb push test/edge/grammar.gql /storage/emulated/0/test/edge/
adb push test/error/errorinfo.gql /storage/emulated/0/test/error/
adb push test/query/grammar.gql /storage/emulated/0/test/query/
adb push test/util/grammar.gql /storage/emulated/0/test/util/
adb push test/vertex/grammar.gql /storage/emulated/0/test/vertex/
adb push test/walk/grammar.gql /storage/emulated/0/test/walk/
adb shell sh /storage/emulated/0/test/vnet
adb shell sh /storage/emulated/0/test/storage
adb shell sh /storage/emulated/0/test/grammar
# adb shell sh /storage/emulated/0/test/query
adb shell sh /storage/emulated/0/test/movielens
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/vertex --outputdir=/storage/emulated/0/test/vertex
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/edge --outputdir=/storage/emulated/0/test/edge
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/error --outputdir=/storage/emulated/0/test/error
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/util --outputdir=/storage/emulated/0/test/util
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/walk --outputdir=/storage/emulated/0/test/walk
