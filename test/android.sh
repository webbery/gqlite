#!/bin/bash

adb root
adb shell mount -o rw,remount /

adb shell mkdir /storage/emulated/0/test
adb shell mkdir /storage/emulated/0/test/edge
adb shell mkdir /storage/emulated/0/test/error
adb shell mkdir /storage/emulated/0/test/graphs
adb shell mkdir /storage/emulated/0/test/query
adb shell mkdir /storage/emulated/0/test/util
adb shell mkdir /storage/emulated/0/test/vertex
adb shell mkdir /storage/emulated/0/test/walk
adb push build/test/vnet /storage/emulated/0/test/vnet
adb push build/test/storage /storage/emulated/0/test/storage
adb push build/test/movielens /storage/emulated/0/test/movielens
adb push build/test/regress /storage/emulated/0/test/regress
adb push build/test/query /storage/emulated/0/test/query
adb push build/test/grammar /storage/emulated/0/test/grammar
adb push test/edge/grammar.gql /storage/emulated/0/test/edge/grammar.gql
adb push test/error/grammar.gql /storage/emulated/0/test/error/grammar.gql
adb push test/graphs/grammar.gql /storage/emulated/0/test/graphs/grammar.gql
adb push test/query/grammar.gql /storage/emulated/0/test/query/grammar.gql
adb push test/util/grammar.gql /storage/emulated/0/test/util/grammar.gql
adb push test/vertex/grammar.gql /storage/emulated/0/test/vertex/grammar.gql
adb push test/walk/grammar.gql /storage/emulated/0/test/walk/grammar.gql
adb shell sh /storage/emulated/0/test/vnet
adb shell sh /storage/emulated/0/test/storage
adb shell sh /storage/emulated/0/test/grammar
adb shell sh /storage/emulated/0/test/query
adb shell sh /storage/emulated/0/test/movielens
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/vertex --outputdir=/storage/emulated/0/test/vertex
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/edge --outputdir=/storage/emulated/0/test/edge
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/error --outputdir=/storage/emulated/0/test/error
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/graphs --outputdir=/storage/emulated/0/test/graphs
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/query --outputdir=/storage/emulated/0/test/query
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/util --outputdir=/storage/emulated/0/test/util
adb shell sh /storage/emulated/0/test/regress --inputdir=/storage/emulated/0/test/walk --outputdir=/storage/emulated/0/test/walk
