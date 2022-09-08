#!/bin/bash

adb shell mount -o rw,remount /

adb shell mkdir /sdcard/test
adb shell mkdir /sdcard/test/edge
adb shell mkdir /sdcard/test/error
adb shell mkdir /sdcard/test/graphs
adb shell mkdir /sdcard/test/query
adb shell mkdir /sdcard/test/util
adb shell mkdir /sdcard/test/vertex
adb shell mkdir /sdcard/test/walk
adb push build/test/vnet /sdcard/test/vnet
adb push build/test/storage /sdcard/test/storage
adb push build/test/movielens /sdcard/test/movielens
adb push build/test/regress /sdcard/test/regress
adb push build/test/query /sdcard/test/query
adb push build/test/grammar /sdcard/test/grammar
adb push test/edge/grammar.gql /sdcard/test/edge/grammar.gql
adb push test/error/grammar.gql /sdcard/test/error/grammar.gql
adb push test/graphs/grammar.gql /sdcard/test/graphs/grammar.gql
adb push test/query/grammar.gql /sdcard/test/query/grammar.gql
adb push test/util/grammar.gql /sdcard/test/util/grammar.gql
adb push test/vertex/grammar.gql /sdcard/test/vertex/grammar.gql
adb push test/walk/grammar.gql /sdcard/test/walk/grammar.gql
adb shell sh /sdcard/test/vnet
adb shell sh /sdcard/test/storage
adb shell sh /sdcard/test/grammar
adb shell sh /sdcard/test/query
adb shell sh /sdcard/test/movielens
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/vertex --outputdir=/sdcard/test/vertex
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/edge --outputdir=/sdcard/test/edge
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/error --outputdir=/sdcard/test/error
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/graphs --outputdir=/sdcard/test/graphs
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/query --outputdir=/sdcard/test/query
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/util --outputdir=/sdcard/test/util
adb shell sh /sdcard/test/regress --inputdir=/sdcard/test/walk --outputdir=/sdcard/test/walk
